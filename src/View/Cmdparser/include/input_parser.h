/**
 * @file input_parser.h
 * @brief 命令行解析器（InputParser）及命令数据结构定义。
 *
 * 基于 CLI11 将用户输入解析为结构化的 Command 对象（点分 verb、
 * 位置参数、命名参数），供派发器路由使用。
 *
 * 依赖：CLI11 第三方库。
 */
#pragma once

#include <CLI/CLI.hpp>

#include <string>
#include <map>
#include <vector>

namespace mud::cmd
{
    /**
     * @brief 解析后的命令结构。
     *
     * verb 为点分动词（如 mine.start）；args 为位置参数；
     * options 为命名参数（--layer -> "2"）；raw 保留原始输入。
     */
    struct Command
    {
        std::string verb; // 点分 verb 如 mine.start
        std::vector<std::string> args; // 位置参数
        std::map<std::string, std::string> options; // 命名参数 --layer -> "2"
        std::string raw;
    };
}

/**
 * @brief 命令行解析器：维护 CLI11 schema 并将输入行转换为 Command。
 */
class InputParser
{
public:
    InputParser();
    mud::cmd::Command parse(const std::string& line);
    std::string help_text() const; // 返回帮助文本

private:
    CLI::App app_;
    std::size_t layer_{0};    // --layer 目标层（0-4）
    double factor_{1.0};      // --factor 时间倍率
    std::size_t plot_{0};     // --plot 地块索引（0 起）
    std::string crop_;        // --crop 作物名（cabbage/carrot/...）
    std::string fert_type_;   // --type 肥料类型（normal/advanced）
    std::string shop_;        // --shop 商店 ID
    std::string item_;        // --item 物品名
    std::size_t count_{1};    // --count 数量（默认 1）
    std::string tool_;        // --tool 工具名（hoe/rod/pickaxe）
    std::string method_;      // --method 修复方式（ore/gold）
};
