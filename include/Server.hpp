
#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <sys/poll.h>

class Client;
class Channel;

class Server {
public:
    Server(int port, const std::string& password);
    ~Server();

    void run(); // main loop

private:
    int                     _port;
    std::string             _password;
    int                     _listenFd;
    std::vector<struct pollfd> _pfds;
    std::map<int, Client*>  _clients; // fd -> Client*
    std::map<std::string, Client*> _nicks; // nick -> Client*
    std::map<std::string, Channel*> _channels; // #name -> Channel*

    void initSocket();
    void addPollFd(int fd, short events);
    void removePollFd(int fd);

    void handleNewConnection();
    void handleRead(int fd);
    void handleWrite(int fd);
    void disconnect(int fd, const std::string& reason);

    void processLine(Client* c, const std::string& line);

    // command handlers (MVP)
    void cmdPASS(Client* c, const std::vector<std::string>& p);
    void cmdNICK(Client* c, const std::vector<std::string>& p);
    void cmdUSER(Client* c, const std::vector<std::string>& p);
    void cmdPING(Client* c, const std::vector<std::string>& p);
    void cmdPONG(Client* c, const std::vector<std::string>& p);
    void cmdJOIN(Client* c, const std::vector<std::string>& p);
    void cmdPART(Client* c, const std::vector<std::string>& p);
    void cmdPRIVMSG(Client* c, const std::vector<std::string>& p);

    void tryRegister(Client* c);

    // helpers
    void sendNumeric(Client* c, const std::string& code, const std::string& msg);
    void sendWelcome(Client* c);
    std::string prefix(Client* c) const;
    void sendRaw(Client* c, const std::string& line);
    Channel* getOrCreateChannel(const std::string& name);
    void partAll(Client* c, const std::string& reason);
};

#endif
