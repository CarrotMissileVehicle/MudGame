/**
 * @file CommandCatalog.h
 * @brief 命令元数据目录：已知动词、帮助文本与参数候选补全。
 *
 * 取自原 main.cpp 的 kKnownVerbs、help 文本与 param_choices，
 * 供 TuiController 的命令收集状态机与动态补全使用。
 */
#pragma once

#include <string>
#include <vector>

#include "connector.h"

class GameContext;

/**
 * @brief 命令目录：静态提供命令集合与交互元数据。
 */
class CommandCatalog
{
public:
    /// 全部已知动词（用于输入补全过滤）。
    static std::vector<std::string> known_verbs();

    /// help 命令的帮助文本。
    static std::string help_text();

    /// 按参数名返回候选补全列表（随输入过滤；plot 依赖农田规模）。
    static std::vector<std::string> param_choices(
        const mud::cmd::ParameterDef& p, const GameContext& ctx);
};