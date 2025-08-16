
#include "Channel.hpp"
#include "Client.hpp"

Channel::Channel(const std::string& name) : _name(name), _topic() {}

const std::string& Channel::name() const { return _name; }
void Channel::setTopic(const std::string& t) { _topic = t; }
const std::string& Channel::topic() const { return _topic; }

bool Channel::add(Client* c, bool opIfFirst) {
    std::pair<std::set<Client*>::iterator, bool> res = _members.insert(c);
    if (res.second) {
        if (_members.size() == 1 && opIfFirst) _ops.insert(c);
        return true;
    }
    return false;
}

void Channel::remove(Client* c) {
    _members.erase(c);
    _ops.erase(c);
}

bool Channel::has(Client* c) const {
    return _members.find(c) != _members.end();
}

bool Channel::empty() const { return _members.empty(); }

bool Channel::isOp(Client* c) const {
    return _ops.find(c) != _ops.end();
}

void Channel::addOp(Client* c) { _ops.insert(c); }
void Channel::removeOp(Client* c) { _ops.erase(c); }

std::string Channel::namesList() const {
    std::string out;
    for (std::set<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
        Client* c = *it;
        if (!out.empty()) out += " ";
        if (isOp(c)) out += "@";
        out += c->nick();
    }
    return out;
}

const std::set<Client*>& Channel::members() const { return _members; }
