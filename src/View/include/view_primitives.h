/**
 * @file view_primitives.h
 * @brief View 层通用渲染原语：分隔线 / 标题栏 / 字段行 / 时间格式化。
 *
 * 全部经由 Renderer 输出；本模块不依赖任何业务类型
 * （仅使用纯时间类型 GameDateTime 做格式化）。
 */
#pragma once

#include <string>

#include "Renderer.h"
#include "game_time.h"

namespace mud::view
{
    /// 输出宽度固定为 60 字符的分隔线。
    void print_separator(Renderer& r, int width = 60, char ch = '-');

    /// 输出包裹式标题（上下分隔线夹标题）。
    void print_title(Renderer& r, const std::string& title, char ch = '=');

    /// 输出 `label: value` 字段行。
    void print_pair(Renderer& r, const std::string& label, const std::string& value);

    /// 统一时间显示格式：`Y年M月D日 HH:MM`（收编 main 的 fmt_time）。
    std::string format_time(const mud::time::GameDateTime& t);

    /// 输出命令提示符 `> `（不换行）。
    void print_prompt(Renderer& r);

    /// 输出输入格式提示行（如 `输入格式：market.buy --shop seed --item 小白菜种子`）。
    /// 组合根负责装配提示文本，View 只负责展示。
    void print_input_hint(Renderer& r, const std::string& hint);
} // namespace mud::view