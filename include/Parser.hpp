
#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <vector>

struct CommandMsg {
    std::string command;             // uppercased
    std::vector<std::string> params; // params (last may contain spaces if trailing)
};

class Parser {
public:
    static bool extractLine(std::string& buffer, std::string& outLine); // pulls a line ending with \r\n
    static CommandMsg parse(const std::string& line);
};

#endif
