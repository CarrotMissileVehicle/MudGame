/**
 * @file parameter_collector.cpp
 * @brief 交互式参数收集器实现。
 *
 * 按 CommandSchema 逐个提示用户输入参数，支持默认值、必填校验和取消操作。
 */
#include "input_parser.h"
#include "Renderer.h"

#include <iostream>
#include <string>

ParameterCollector::ParameterCollector(
    mud::view::Renderer& renderer,
    std::function<std::string()> input_fn)
    : renderer_(renderer),
      input_fn_(input_fn ? std::move(input_fn)
                         : []() {
                               std::string line;
                               std::getline(std::cin, line);
                               // 过滤管道/文件输入可能携带的残留回车符
                               if (!line.empty() && line.back() == '\r')
                                   line.pop_back();
                               return line;
                           })
{
}

bool ParameterCollector::collect(const mud::cmd::CommandSchema& schema,
                                 mud::cmd::Command& cmd)
{
    for (const auto& param : schema.parameters)
    {
        // 已由传统解析提供，跳过
        if (cmd.options.count(param.name)) continue;

        while (true)
        {
            // 显示提示
            if (!param.required && param.default_value.empty())
                renderer_.print(param.prompt + "（可选，直接回车跳过）：");
            else
                renderer_.print(param.prompt + "：");

            const std::string input = input_fn_();

            // 用户输入 q → 取消整个操作
            if (input == "q" || input == "Q")
                return false;

            if (input.empty())
            {
                if (!param.default_value.empty())
                {
                    // 有默认值，使用默认值
                    cmd.options[param.name] = param.default_value;
                    break;
                }
                if (param.required)
                {
                    renderer_.print("此参数为必填项，请重新输入。");
                    continue; // 重新提示
                }
                // 非必填且无默认值，跳过
                break;
            }

            // 有输入，记录并继续下一个参数
            cmd.options[param.name] = input;
            break;
        }
    }
    return true;
}
