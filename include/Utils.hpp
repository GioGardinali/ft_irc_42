
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace Utils {
    std::string trim(const std::string& s);
    std::vector<std::string> split(const std::string& s, char delim);
    std::string toUpper(const std::string& s);
    bool isNumber(const std::string& s);
}

#endif
