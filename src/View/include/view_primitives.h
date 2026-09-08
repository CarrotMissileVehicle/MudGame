/**
 * @file view_primitives.h
 * @brief View 层通用渲染原语：分隔线 / 标题 / 键值对 / 时间格式化 / 提示符。
 *
 * 仅依赖 Renderer 抽象与纯时间类型，不触碰任何业务模块。
 */
#pragma once

#include <string>

#include "Renderer.h"
#include "game_time.h"

namespace mud::view
{
    /** @brief 输出宽度个 ch 字符组成的分隔线。 */
    void print_separator(Renderer& r, int width, char ch);

    /** @brief 输出标题（上下各一条 ch 组成的边线）。 */
    void print_title(Renderer& r, const std::string& title, char ch);

    /** @brief 输出 "label: value" 形式的键值对。 */
    void print_pair(Renderer& r, const std::string& label, const std::string& value);

    /** @brief 格式化为 "Y年M月D日 HH:MM"（时分补零）。 */
    [[nodiscard]] std::string format_time(const mud::time::GameDateTime& t);

    /** @brief 输出命令提示符。 */
    void print_prompt(Renderer& r);

    /** @brief 输出输入格式提示行。 */
    void print_input_hint(Renderer& r, const std::string& hint);
} // namespace mud::view
