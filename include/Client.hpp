
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <set>
#include <vector>

class Client {
public:
    enum State { NEW, PASS_OK, REGISTERED };

    Client(int fd);
    ~Client();

    int         getFd() const;
    std::string& inBuf();
    std::string& outBuf();

    // identity
    void    setPasswordOK(bool ok);
    bool    passwordOK() const;
    void    setNick(const std::string& n);
    void    setUser(const std::string& u);
    void    setRealName(const std::string& r);
    const std::string& nick() const;
    const std::string& user() const;
    const std::string& realname() const;
    bool    isRegistered() const;
    void    setRegistered(bool v);

    void    enqueue(const std::string& data);

    // Channels
    void joinChannel(const std::string& ch);
    void leaveChannel(const std::string& ch);
    const std::set<std::string>& channelsJoined() const;

private:
    int         _fd;
    std::string _in;
    std::string _out;

    bool        _passOK;
    bool        _registered;
    std::string _nick;
    std::string _user;
    std::string _realname;
    std::set<std::string> _channels;
};

#endif
