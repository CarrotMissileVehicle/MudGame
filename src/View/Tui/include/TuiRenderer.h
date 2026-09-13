/**
 * @file TuiRenderer.h
 * @brief 将 View 层输出重定向到 TUI 日志区的 Renderer 实现。
 *
 * 组合根用它构造 TerminalView，使所有命令反馈/面板输出进入 FTXUI 日志区。
 * 主线程读写 TuiState::logs，安全。
 */
#pragma once

#include <string>

#include "Renderer.h"

namespace mud::view { class TuiRenderer; }

namespace mud::tui
{
    struct TuiState;

    /** @brief 把渲染文本追加到 TUI 日志区。 */
    class TuiRenderer : public mud::view::Renderer
    {
    public:
        explicit TuiRenderer(TuiState& state) : state_(state) {}

        void print(const std::string& line) override;
        void print_raw(const std::string& s) override;

    private:
        TuiState& state_;
    };
} // namespace mud::tui
