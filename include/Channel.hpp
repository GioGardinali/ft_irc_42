
#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <map>

class Client;

class Channel {
public:
    Channel(const std::string& name);

    const std::string& name() const;
    void setTopic(const std::string& t);
    const std::string& topic() const;

    bool add(Client* c, bool opIfFirst);
    void remove(Client* c);
    bool has(Client* c) const;
    bool empty() const;

    // ops
    bool isOp(Client* c) const;
    void addOp(Client* c);
    void removeOp(Client* c);

    // Build names list (with @ for ops)
    std::string namesList() const;

    // Iterate members (read-only)
    const std::set<Client*>& members() const;

private:
    std::string         _name;
    std::string         _topic;
    std::set<Client*>   _members;
    std::set<Client*>   _ops;
};

#endif
