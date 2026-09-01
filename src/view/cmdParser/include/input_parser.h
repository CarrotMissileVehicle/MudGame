#pragma once

#include <CLI/CLI.hpp>

#include <string>
#include <map>
#include <vector>

namespace mud::cmd
{
    struct Command
    {
        std::string verb; // 点分 verb 如 mine.start
        std::vector<std::string> args; // 位置参数
        std::map<std::string, std::string> options; // 命名参数 --layer -> "2"
        std::string raw;
    };
}

class InputParser
{
public:
    InputParser();
    mud::cmd::Command parse(const std::string& line);
    std::string help_text() const; // 返回帮助文本

private:
    CLI::App app_;
    std::size_t layer_{0}; // --layer 目标层（0-4）
    double factor_{1.0}; // --factor 时间倍率
};
