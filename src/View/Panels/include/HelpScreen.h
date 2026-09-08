/**
 * @file HelpScreen.h
 * @brief 帮助屏：渲染命令帮助文本（收编 main 的 parser.help_text() 输出）。
 */
#pragma once

#include <string>

#include "Renderer.h"

namespace mud::view
{
    class HelpScreen
    {
    public:
        explicit HelpScreen(Renderer& r) : r_(r) {}

        /// 渲染完整帮助文本（整块输出，不追加换行之外的修饰）。
        void render(const std::string& help_text) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view