/**
 * @file input_parser.h
 * @brief 命令行解析器（InputParser）及命令数据结构定义。
 *
 * 两种解析模式：
 *   1. 传统模式：基于 CLI11 将完整命令行解析为 Command 对象
 *   2. 交互模式：仅从输入行提取 verb，参数由 ParameterCollector 逐个提示收集
 *
 * 依赖：CLI11 第三方库。
 */
#pragma once

#include <CLI/CLI.hpp>

#include <functional>
#include <string>
#include <map>
#include <vector>

#include "Renderer.h"

namespace mud::cmd
{
    /**
     * @brief 数值参数安全解析：文本须为完整合法 double。
     *
     * 容忍首尾空白（交互输入残留），拒绝 nan/inf 与尾部杂字符；
     * 失败返回 false 且不修改 out。供命令处理器替代裸 try/catch
     * std::stod（DEF-102 下沉：非法倍率不再令 REPL 崩溃）。
     */
    bool parse_double(const std::string& text, double& out);

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

    /**
     * @brief 单个参数的定义（交互模式下使用）。
     */
    struct ParameterDef
    {
        std::string name;         // 参数名（存入 Command.options 的 key）
        std::string prompt;       // 提示文本（如 "地块索引(0-3)"）
        bool required = true;     // 是否必填
        std::string default_value; // 默认值（非空时可直接回车跳过）
    };

    /**
     * @brief 命令的参数 Schema：描述一个命令需要哪些参数。
     */
    struct CommandSchema
    {
        std::string description;                    // 命令描述
        std::vector<ParameterDef> parameters;       // 参数定义列表（按收集顺序）
    };
}

/**
 * @brief 命令行解析器：维护 CLI11 schema 并将输入行转换为 Command。
 */
class InputParser
{
public:
    InputParser();

    /** @brief 传统解析：完整命令行（含参数）→ Command。 */
    mud::cmd::Command parse(const std::string& line);

    /** @brief 交互模式：仅从输入行提取 verb（忽略后续参数）。 */
    std::string parse_verb_only(const std::string& line) const;

    /** @brief 返回 CLI11 生成的完整帮助文本。 */
    std::string help_text() const;

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

/**
 * @brief 交互式参数收集器：按 CommandSchema 逐个提示用户输入参数。
 *
 * 用法：用户输入命令动词后，ParameterCollector 遍历 Schema 中的参数定义，
 * 对每个参数输出提示文本并读取用户输入，验证后填入 Command::options。
 * 支持默认值（直接回车使用）、必填校验和取消操作（输入 q）。
 */
class ParameterCollector
{
public:
    /**
     * @brief 构造函数。
     * @param renderer 输出渲染器。
     * @param input_fn 输入读取函数（默认从 std::cin 读取一行）。
     */
    explicit ParameterCollector(
        mud::view::Renderer& renderer,
        std::function<std::string()> input_fn = nullptr);

    /**
     * @brief 逐个提示并收集命令所需的参数。
     * @param schema 命令的参数定义。
     * @param cmd 待填充的命令对象（verb 已设置）。
     * @return true 收集成功；false 用户取消（输入 q）。
     */
    bool collect(const mud::cmd::CommandSchema& schema, mud::cmd::Command& cmd);

private:
    mud::view::Renderer& renderer_;
    std::function<std::string()> input_fn_;
};
