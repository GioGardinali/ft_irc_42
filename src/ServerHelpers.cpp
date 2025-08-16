
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

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
