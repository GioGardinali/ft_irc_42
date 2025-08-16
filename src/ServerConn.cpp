
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <stdexcept>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

void Server::initSocket() {
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0) throw std::runtime_error("socket() failed");

    int yes = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        throw std::runtime_error("setsockopt() failed");
    }
    int flags = fcntl(_listenFd, F_GETFL, 0);
    if (flags < 0 || fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("fcntl() failed");
    }

    struct sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");
    if (listen(_listenFd, 64) < 0)
        throw std::runtime_error("listen() failed");

    _pfds.clear();
    addPollFd(_listenFd, POLLIN);
}

void Server::addPollFd(int fd, short events) {
    struct pollfd p;
    p.fd = fd;
    p.events = events;
    p.revents = 0;
    _pfds.push_back(p);
}

void Server::removePollFd(int fd) {
    for (size_t i = 0; i < _pfds.size(); ++i) {
        if (_pfds[i].fd == fd) { _pfds.erase(_pfds.begin() + i); return; }
    }
}

void Server::handleNewConnection() {
    while (true) {
        struct sockaddr_in cli; socklen_t clilen = sizeof(cli);
        int cfd = accept(_listenFd, (struct sockaddr*)&cli, &clilen);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            else { std::perror("accept"); break; }
        }
        int flags = fcntl(cfd, F_GETFL, 0);
        fcntl(cfd, F_SETFL, flags | O_NONBLOCK);

        Client* c = new Client(cfd);
        _clients[cfd] = c;
        addPollFd(cfd, POLLIN);
    }
}
