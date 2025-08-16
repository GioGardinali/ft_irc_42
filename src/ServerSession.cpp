
#include "Server.hpp"
#include "Client.hpp"
#include "Replies.hpp"
#include "Channel.hpp"
#include <unistd.h>
#include <cstring>

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
        if (it->second->empty()) { delete it->second; _channels.erase(it++); }
        else ++it;
    }
}
