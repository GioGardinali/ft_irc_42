
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Replies.hpp"

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
