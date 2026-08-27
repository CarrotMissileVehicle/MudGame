#pragma once

#include <string>
#include <map>
#include <vector>

namespace mud::cmd {
    struct Command {
        std::string verb;                              // 首词
        std::vector<std::string> args;                 // 位置参数
        std::map<std::string,std::string> options;     // 命名参数
        std::string raw;
    };
}

class InputParser {
public:
    mud::cmd::Command parse(const std::string& line) const;   // throws ParseError
};