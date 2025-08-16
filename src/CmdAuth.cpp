
#include "Server.hpp"
#include "Client.hpp"
#include "Replies.hpp"
#include "Utils.hpp"
#include <cctype>

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

    // 1) começa com letra
    if (nick.empty() || !std::isalpha((unsigned char)nick[0])) {
        sendRaw(c, Replies::ERR_ERRONEUSNICKNAME(c->nick(), nick));
        return;
    }

    // 2) caracteres permitidos no restante: [A-Za-z0-9_\-\[\]\\`^{}]
    for (size_t i = 1; i < nick.size(); ++i) {
        unsigned char ch = (unsigned char)nick[i];
        if (!(std::isalnum(ch) || ch=='_' || ch=='-' || ch=='[' || ch==']'
              || ch=='\\' || ch=='`' || ch=='^' || ch=='{' || ch=='}')) {
            sendRaw(c, Replies::ERR_ERRONEUSNICKNAME(c->nick(), nick));
            return;
        }
    }

    // 3) conflito de nick
    std::map<std::string, Client*>::iterator it = _nicks.find(nick);
    if (it != _nicks.end() && it->second != c) {
        sendRaw(c, Replies::ERR_NICKNAMEINUSE(c->nick(), nick));
        return;
    }

    // 4) aplica
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
