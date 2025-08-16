
#include "Server.hpp"
#include "Client.hpp"
#include "Parser.hpp"
#include "Utils.hpp"
#include "Replies.hpp"
#include "Channel.hpp"
#include <cerrno>
#include <cstdio>
#include <unistd.h>

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
