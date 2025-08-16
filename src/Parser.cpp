
#include "Parser.hpp"
#include "Utils.hpp"

bool Parser::extractLine(std::string& buffer, std::string& outLine) {
    size_t pos = buffer.find("\r\n");
    size_t use = std::string::npos;
    if (pos != std::string::npos) use = pos;
    else {
        pos = buffer.find('\n');
        if (pos != std::string::npos) use = pos;
    }
    if (use == std::string::npos) return false;
    outLine = buffer.substr(0, use);
    if (buffer.size() >= use + 2 && buffer[use] == '\r' && buffer[use+1] == '\n')
        buffer.erase(0, use + 2);
    else
        buffer.erase(0, use + 1);
    return true;
}

CommandMsg Parser::parse(const std::string& raw) {
    CommandMsg m;
    std::string line = raw;
    if (!line.empty() && line[0] == ':') {
        size_t sp = line.find(' ');
        if (sp != std::string::npos) line = line.substr(sp + 1);
    }
    std::vector<std::string> tokens;
    std::string current;
    bool trailing = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (!trailing) {
            if (c == ' ') {
                if (!current.empty()) { tokens.push_back(current); current.clear(); }
            } else if (c == ':') {
                if (!current.empty()) { tokens.push_back(current); current.clear(); }
                std::string rest = line.substr(i + 1);
                tokens.push_back(rest);
                trailing = true;
                break;
            } else current += c;
        } else {
            current += c;
        }
    }
    if (!current.empty() && !trailing) tokens.push_back(current);
    if (tokens.empty()) return m;
    m.command = Utils::toUpper(tokens[0]);
    for (size_t i = 1; i < tokens.size(); ++i) m.params.push_back(tokens[i]);
    return m;
}
