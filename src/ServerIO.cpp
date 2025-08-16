
#include "Server.hpp"
#include "Client.hpp"
#include "Parser.hpp"
#include <cerrno>
#include <sys/socket.h>
#include <unistd.h>

void Server::handleRead(int fd) {
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) return;
        disconnect(fd, "read error/close"); return;
    }
    Client* c = _clients[fd];
    c->inBuf().append(buf, n);

    std::string line;
    while (Parser::extractLine(c->inBuf(), line)) {
        if (line.size() > 512) line = line.substr(0, 512);
        processLine(c, line);
    }
}

void Server::handleWrite(int fd) {
    Client* c = _clients[fd];
    std::string& out = c->outBuf();
    if (out.empty()) return;
    ssize_t n = send(fd, out.c_str(), out.size(), 0);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) return;
        disconnect(fd, "write error"); return;
    }
    out.erase(0, (size_t)n);
    if (out.empty()) {
        for (size_t i = 0; i < _pfds.size(); ++i) if (_pfds[i].fd == fd) _pfds[i].events &= ~POLLOUT;
    }
}

void Server::disconnect(int fd, const std::string& /*reason*/) {
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        partAll(it->second, "Client quit");
        Client* c = it->second;
        if (!c->nick().empty()) _nicks.erase(c->nick());
        delete c;
        _clients.erase(it);
    }
    removePollFd(fd);
    close(fd);
}
