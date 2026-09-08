/**
 * @file parameter_collector.cpp
 * @brief 交互式参数收集器实现（ParameterCollector）。
 *
 * 按 CommandSchema 逐个提示用户输入参数：支持默认值（直接回车）、
 * 必填校验与取消操作（输入 q/Q）。收集结果填入 Command::options。
 */
#include "input_parser.h"

#include <iostream>
#include <string>
#include <utility>

namespace
{
    /// 默认输入函数：从 std::cin 读取一行（剥离末尾换行）。
    std::string cin_input()
    {
        std::string line;
        if (!std::getline(std::cin, line)) return "";
        return line;
    }
}

ParameterCollector::ParameterCollector(
    mud::view::Renderer& renderer,
    std::function<std::string()> input_fn)
    : renderer_(renderer), input_fn_(input_fn ? std::move(input_fn) : cin_input)
{
}

bool ParameterCollector::collect(const mud::cmd::CommandSchema& schema, mud::cmd::Command& cmd)
{
    for (const auto& param : schema.parameters)
    {
        // 已有值的参数直接跳过（传统模式或已通过命令行提供）
        if (cmd.options.count(param.name)) continue;

        std::string prompt = "[" + param.name + "] " + param.prompt;
        if (!param.default_value.empty())
            prompt += "（默认 " + param.default_value + "，直接回车使用）";
        renderer_.print_raw(prompt + "：> ");

        std::string input = input_fn_();
        if (input == "q" || input == "Q")
            return false; // 用户取消

        if (input.empty() && !param.default_value.empty())
            input = param.default_value;

        if (input.empty())
        {
            if (param.required)
            {
                renderer_.print("参数 [" + param.name + "] 必填，已取消该命令。");
                return false;
            }
            continue; // 可选参数留空
        }
        cmd.options[param.name] = input;
    }
    return true;
}