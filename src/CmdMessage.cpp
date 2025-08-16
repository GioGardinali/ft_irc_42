
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

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
