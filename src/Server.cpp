
#include "Server.hpp"
#include "Client.hpp"
#include "Parser.hpp"
#include "Utils.hpp"
#include "Replies.hpp"
#include "Channel.hpp"

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

Server::Server(int port, const std::string& password)
: _port(port), _password(password), _listenFd(-1) {
    initSocket();
}

Server::~Server() {
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        close(it->first);
        delete it->second;
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete it->second;
    }
    if (_listenFd >= 0) close(_listenFd);
}

void Server::initSocket() {
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0) throw std::runtime_error("socket() failed");

    int yes = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        throw std::runtime_error("setsockopt() failed");
    }
    int flags = fcntl(_listenFd, F_GETFL, 0);
    if (flags < 0 || fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("fcntl() failed");
    }

    struct sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");
    if (listen(_listenFd, 64) < 0)
        throw std::runtime_error("listen() failed");

    _pfds.clear();
    addPollFd(_listenFd, POLLIN);
}

void Server::addPollFd(int fd, short events) {
    struct pollfd p;
    p.fd = fd;
    p.events = events;
    p.revents = 0;
    _pfds.push_back(p);
}

void Server::removePollFd(int fd) {
    for (size_t i = 0; i < _pfds.size(); ++i) {
        if (_pfds[i].fd == fd) { _pfds.erase(_pfds.begin() + i); return; }
    }
}

void Server::sendRaw(Client* c, const std::string& line) {
    c->enqueue(line);
    for (size_t i = 0; i < _pfds.size(); ++i) if (_pfds[i].fd == c->getFd()) _pfds[i].events |= POLLOUT;
}

std::string Server::prefix(Client* c) const {
    std::string host = "localhost";
    return ":" + c->nick() + "!" + c->user() + "@" + host;
}

Channel* Server::getOrCreateChannel(const std::string& name) {
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end()) return it->second;
    Channel* ch = new Channel(name);
    _channels[name] = ch;
    return ch;
}

void Server::partAll(Client* c, const std::string& reason) {
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        Channel* ch = it->second;
        if (ch->has(c)) {
            std::string line = prefix(c) + " QUIT :" + reason + "\r\n";
            const std::set<Client*>& mem = ch->members();
            for (std::set<Client*>::const_iterator mit = mem.begin(); mit != mem.end(); ++mit) {
                if (*mit == c) continue;
                sendRaw(*mit, line);
            }
            ch->remove(c);
        }
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ) {
        if (it->second->empty()) { delete it->second; it = _channels.erase(it); }
        else ++it;
    }
}

void Server::run() {
    while (true) {
        int ret = poll(&_pfds[0], _pfds.size(), -1);
        if (ret < 0) {
            if (errno == EINTR) continue;
            std::perror("poll"); break;
        }
        for (size_t i = 0; i < _pfds.size(); ++i) {
            struct pollfd p = _pfds[i];
            if (p.revents == 0) continue;
            if (p.fd == _listenFd && (p.revents & POLLIN)) {
                handleNewConnection();
            } else {
                if (p.revents & POLLIN) handleRead(p.fd);
                if (p.revents & POLLOUT) handleWrite(p.fd);
                if (p.revents & (POLLERR | POLLHUP | POLLNVAL)) disconnect(p.fd, "poll error");
            }
        }
    }
}

void Server::handleNewConnection() {
    while (true) {
        struct sockaddr_in cli; socklen_t clilen = sizeof(cli);
        int cfd = accept(_listenFd, (struct sockaddr*)&cli, &clilen);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            else { std::perror("accept"); break; }
        }
        int flags = fcntl(cfd, F_GETFL, 0);
        fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

        Client* c = new Client(cfd);
        _clients[cfd] = c;
        addPollFd(cfd, POLLIN);
    }
}

void Server::handleRead(int fd) {
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
        disconnect(fd, "read error/close"); return;
    }
    Client* c = _clients[fd];
    c->inBuf().append(buf, n);

    std::string line;
    while (Parser::extractLine(c->inBuf(), line)) {
        if (line.size() > 512) line = line.substr(0, 512);
        processLine(c, line);
    }
}

void Server::handleWrite(int fd) {
    Client* c = _clients[fd];
    std::string& out = c->outBuf();
    if (out.empty()) return;
    ssize_t n = send(fd, out.c_str(), out.size(), 0);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        disconnect(fd, "write error"); return;
    }
    out.erase(0, (size_t)n);
    if (out.empty()) {
        for (size_t i = 0; i < _pfds.size(); ++i) if (_pfds[i].fd == fd) _pfds[i].events &= ~POLLOUT;
    }
}

void Server::disconnect(int fd, const std::string& /*reason*/) {
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        partAll(it->second, "Client quit");
        Client* c = it->second;
        if (!c->nick().empty()) _nicks.erase(c->nick());
        delete c;
        _clients.erase(it);
    }
    removePollFd(fd);
    close(fd);
}

void Server::processLine(Client* c, const std::string& line) {
    CommandMsg m = Parser::parse(line);
    if (m.command.empty()) return;
    if (m.command == "PASS") cmdPASS(c, m.params);
    else if (m.command == "NICK") cmdNICK(c, m.params);
    else if (m.command == "USER") cmdUSER(c, m.params);
    else if (m.command == "PING") cmdPING(c, m.params);
    else if (m.command == "PONG") cmdPONG(c, m.params);
    else if (m.command == "QUIT") { disconnect(c->getFd(), "quit"); }
    else if (m.command == "JOIN") cmdJOIN(c, m.params);
    else if (m.command == "PART") cmdPART(c, m.params);
    else if (m.command == "PRIVMSG") cmdPRIVMSG(c, m.params);
    else {
        c->enqueue(Replies::ERR_UNKNOWNCOMMAND(c->nick(), m.command));
        for (size_t i = 0; i < _pfds.size(); ++i) if (_pfds[i].fd == c->getFd()) _pfds[i].events |= POLLOUT;
    }
}

void Server::sendNumeric(Client* c, const std::string& code, const std::string& msg) {
    std::string nick = c->nick().empty() ? "*" : c->nick();
    std::string line;
    if (msg.size() >= 2 && msg[msg.size()-2] == '\r' && msg[msg.size()-1] == '\n') {
        line = ":" + std::string("ft_irc") + " " + code + " " + nick + " " + msg;
    } else {
        line = ":" + std::string("ft_irc") + " " + code + " " + nick + " " + msg + "\r\n";
    }
    sendRaw(c, line);
}

void Server::sendWelcome(Client* c) {
    char hostbuf[256]; hostbuf[0] = 0;
    if (gethostname(hostbuf, sizeof(hostbuf)-1) != 0) std::strcpy(hostbuf, "localhost");
    std::string host = hostbuf;

    sendRaw(c, Replies::RPL_WELCOME(c->nick(), c->user(), host));
    sendRaw(c, Replies::RPL_YOURHOST(c->nick()));
    sendRaw(c, Replies::RPL_CREATED(c->nick()));
    sendRaw(c, Replies::RPL_MYINFO(c->nick()));
    sendRaw(c, Replies::RPL_MOTDSTART(c->nick()));
    sendRaw(c, Replies::RPL_MOTD(c->nick(), "Welcome! Minimal ft_irc MVP"));
    sendRaw(c, Replies::RPL_ENDOFMOTD(c->nick()));
}

void Server::tryRegister(Client* c) {
    if (!c->passwordOK()) return;
    if (c->nick().empty() || c->user().empty()) return;
    if (c->isRegistered()) return;
    c->setRegistered(true);
    sendWelcome(c);
}

void Server::cmdPASS(Client* c, const std::vector<std::string>& p) {
    if (c->isRegistered()) { sendRaw(c, Replies::ERR_ALREADYREGISTRED(c->nick())); return; }
    if (p.empty()) { sendRaw(c, Replies::ERR_NEEDMOREPARAMS(c->nick(), "PASS")); return; }
    std::string pwd = p[0];
    if (pwd == _password) c->setPasswordOK(true);
    else { sendRaw(c, Replies::ERR_PASSWDMISMATCH(c->nick())); }
    tryRegister(c);
}

void Server::cmdNICK(Client* c, const std::vector<std::string>& p) {
    if (p.empty()) { sendRaw(c, Replies::ERR_NONICKNAMEGIVEN(c->nick())); return; }
    std::string nick = p[0];
    if (nick.empty() || !std::isalpha((unsigned char)nick[0])) { sendRaw(c, Replies::ERR_ERRONEUSNICKNAME(c->nick(), nick)); return; }
    for (size_t i = 1; i < nick.size(); ++i) {
        unsigned char ch = (unsigned char)nick[i];
        if (!(std::isalnum(ch) || ch=='_' || ch=='-' || ch=='[' || ch==']' || ch=='\' || ch=='`' || ch=='^' || ch=='{' || ch=='}')) {
            sendRaw(c, Replies::ERR_ERRONEUSNICKNAME(c->nick(), nick)); return;
        }
    }
    if (_nicks.find(nick) != _nicks.end() && _nicks[nick] != c) {
        sendRaw(c, Replies::ERR_NICKNAMEINUSE(c->nick(), nick)); return;
    }
    if (!c->nick().empty()) _nicks.erase(c->nick());
    c->setNick(nick);
    _nicks[nick] = c;
    tryRegister(c);
}

void Server::cmdUSER(Client* c, const std::vector<std::string>& p) {
    if (c->isRegistered()) { sendRaw(c, Replies::ERR_ALREADYREGISTRED(c->nick())); return; }
    if (p.size() < 4) { sendRaw(c, Replies::ERR_NEEDMOREPARAMS(c->nick(), "USER")); return; }
    c->setUser(p[0]);
    c->setRealName(p[3]);
    tryRegister(c);
}

void Server::cmdPING(Client* c, const std::vector<std::string>& p) {
    if (p.empty()) { sendRaw(c, Replies::ERR_NOORIGIN(c->nick())); return; }
    std::string resp = ":"; resp += "ft_irc PONG ft_irc ";
    resp += ":" + p[0] + "\r\n";
    sendRaw(c, resp);
}

void Server::cmdPONG(Client* /*c*/, const std::vector<std::string>& /*p*/) {}

void Server::cmdJOIN(Client* c, const std::vector<std::string>& p) {
    if (!c->isRegistered()) { sendRaw(c, Replies::ERR_NOTREGISTERED(c->nick())); return; }
    if (p.empty()) { sendRaw(c, Replies::ERR_NEEDMOREPARAMS(c->nick(), "JOIN")); return; }
    std::string chan = p[0];
    if (chan.empty() || chan[0] != '#') { sendRaw(c, Replies::ERR_NOSUCHCHANNEL(c->nick(), chan)); return; }

    Channel* ch = getOrCreateChannel(chan);
    bool added = ch->add(c, true);
    if (!added) {
        // already in channel -> ignore
    } else {
        c->joinChannel(chan);
        std::string j = prefix(c) + " JOIN :" + chan + "\r\n";
        const std::set<Client*>& mem = ch->members();
        for (std::set<Client*>::const_iterator it = mem.begin(); it != mem.end(); ++it) sendRaw(*it, j);
        sendRaw(c, Replies::RPL_NAMREPLY(c->nick(), chan, ch->namesList()));
        sendRaw(c, Replies::RPL_ENDOFNAMES(c->nick(), chan));
    }
}

void Server::cmdPART(Client* c, const std::vector<std::string>& p) {
    if (!c->isRegistered()) { sendRaw(c, Replies::ERR_NOTREGISTERED(c->nick())); return; }
    if (p.empty()) { sendRaw(c, Replies::ERR_NEEDMOREPARAMS(c->nick(), "PART")); return; }
    std::string chan = p[0];
    std::map<std::string, Channel*>::iterator it = _channels.find(chan);
    if (it == _channels.end()) { sendRaw(c, Replies::ERR_NOSUCHCHANNEL(c->nick(), chan)); return; }
    Channel* ch = it->second;
    if (!ch->has(c)) { sendRaw(c, Replies::ERR_NOTONCHANNEL(c->nick(), chan)); return; }

    std::string reason = (p.size() >= 2 ? p[1] : "leaving");
    std::string line = prefix(c) + " PART " + chan + " :" + reason + "\r\n";

    const std::set<Client*>& mem = ch->members();
    for (std::set<Client*>::const_iterator mit = mem.begin(); mit != mem.end(); ++mit) sendRaw(*mit, line);

    ch->remove(c);
    c->leaveChannel(chan);
    if (ch->empty()) { delete ch; _channels.erase(it); }
}

void Server::cmdPRIVMSG(Client* c, const std::vector<std::string>& p) {
    if (!c->isRegistered()) { sendRaw(c, Replies::ERR_NOTREGISTERED(c->nick())); return; }
    if (p.empty()) { sendRaw(c, Replies::ERR_NORECIPIENT(c->nick(), "PRIVMSG")); return; }
    std::string target = p[0];
    if (p.size() < 2) { sendRaw(c, Replies::ERR_NOTEXTTOSEND(c->nick())); return; }
    std::string text = p[1];

    std::string line = prefix(c) + " PRIVMSG " + target + " :" + text + "\r\n";

    if (!target.empty() && target[0] == '#') {
        std::map<std::string, Channel*>::iterator it = _channels.find(target);
        if (it == _channels.end()) { sendRaw(c, Replies::ERR_NOSUCHCHANNEL(c->nick(), target)); return; }
        Channel* ch = it->second;
        if (!ch->has(c)) { sendRaw(c, Replies::ERR_CANNOTSENDTOCHAN(c->nick(), target)); return; }
        const std::set<Client*>& mem = ch->members();
        for (std::set<Client*>::const_iterator mit = mem.begin(); mit != mem.end(); ++mit) {
            if (*mit == c) continue;
            sendRaw(*mit, line);
        }
    } else {
        std::map<std::string, Client*>::iterator it = _nicks.find(target);
        if (it == _nicks.end()) { sendRaw(c, Replies::ERR_NOSUCHNICK(c->nick(), target)); return; }
        sendRaw(it->second, line);
    }
}
