
#include "Utils.hpp"
#include <cctype>
#include <sstream>

std::string Utils::trim(const std::string& s) {
    size_t b = 0;
    while (b < s.size() && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) b++;
    size_t e = s.size();
    while (e > b && (s[e-1] == ' ' || s[e-1] == '\t' || s[e-1] == '\r' || s[e-1] == '\n')) e--;
    return s.substr(b, e - b);
}

std::vector<std::string> Utils::split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == delim) {
            out.push_back(cur);
            cur.clear();
        } else cur += s[i];
    }
    out.push_back(cur);
    return out;
}

std::string Utils::toUpper(const std::string& s) {
    std::string r(s);
    for (size_t i = 0; i < r.size(); ++i) r[i] = std::toupper((unsigned char)r[i]);
    return r;
}

bool Utils::isNumber(const std::string& s) {
    if (s.empty()) return false;
    for (size_t i = 0; i < s.size(); ++i) if (!std::isdigit((unsigned char)s[i])) return false;
    return true;
}
