
#include "Server.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstdlib>

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " <port> <password>\n";
}

int main(int argc, char** argv) {
    if (argc != 3) { usage(argv[0]); return 1; }
    std::string sport = argv[1];
    std::string password = argv[2];
    if (!Utils::isNumber(sport)) {
        std::cerr << "Error: port must be a number\n";
        return 1;
    }
    int port = std::atoi(sport.c_str());
    try {
        Server srv(port, password);
        std::cout << "ft_irc listening on port " << port << " ...\n";
        srv.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
