/**
 * @file LogView.cpp
 * @brief 可滚动日志视图组件实现。
 */
#include "LogView.h"

#include <algorithm>

#include <ftxui/component/mouse.hpp>

#include "TuiState.h"

namespace mud::tui
{
    namespace
    {
        /** @brief 每次滚轮事件的滚动行数。 */
        constexpr int kWheelStep = 3;
    }

    LogView::LogView(TuiState& state) : state_(state) {}

    void LogView::force_follow()
    {
        auto_follow_ = true;
        follow_bottom();
    }

    bool LogView::OnEvent(ftxui::Event event)
    {
        if (event.is_mouse())
        {
            // 尊重全局鼠标捕获；滚轮仅在光标位于日志区域内时生效
            if (!CaptureMouse(event))
                return false;
            const auto& mouse = event.mouse();
            if (!box_.Contain(mouse.x, mouse.y))
                return false;

            if (mouse.button == ftxui::Mouse::WheelUp)
            {
                scroll_by(-kWheelStep);
                return true;
            }
            if (mouse.button == ftxui::Mouse::WheelDown)
            {
                scroll_by(kWheelStep);
                return true;
            }
            return false;
        }

        // 键盘滚动：仅当本组件获得焦点时生效（默认焦点在输入框，不会触发）
        if (!Focused())
            return false;

        const int old = scroll_;
        if (event == ftxui::Event::ArrowUp)
            scroll_by(-1);
        else if (event == ftxui::Event::ArrowDown)
            scroll_by(1);
        else if (event == ftxui::Event::PageUp)
            scroll_by(-std::max(viewport_height(), 1));
        else if (event == ftxui::Event::PageDown)
            scroll_by(std::max(viewport_height(), 1));
        else if (event == ftxui::Event::Home)
        {
            scroll_ = 0;
            auto_follow_ = false;
        }
        else if (event == ftxui::Event::End)
        {
            force_follow();
        }
        else
        {
            return false;
        }

        return old != scroll_;
    }

    ftxui::Element LogView::OnRender()
    {
        // 日志条数或视口尺寸变化时，保证滚动偏移仍在合法范围内
        const bool logs_changed = state_.logs.size() != seen_logs_;
        if (logs_changed)
            seen_logs_ = state_.logs.size();
        scroll_ = std::clamp(scroll_, 0, max_scroll());

        // 检测新日志到达：跟随模式下自动滚到最下方
        if (logs_changed && auto_follow_)
            follow_bottom();

        ftxui::Elements lines;
        lines.reserve(state_.logs.size() + 1);
        for (const auto& line : state_.logs)
            lines.push_back(ftxui::text(line));
        if (lines.empty())
            lines.push_back(ftxui::text("（暂无日志）") | ftxui::dim);

        // 用 focusPosition 精确控制 frame 的可见窗口：
        // 聚焦点 = 当前滚动偏移 + 半视口高度，使第 scroll_ 行位于视口顶部。
        const int total = static_cast<int>(lines.size());
        const int focus_y =
            std::clamp(scroll_ + viewport_height() / 2, 0, std::max(total - 1, 0));

        return ftxui::vbox(std::move(lines))
             | ftxui::focusPosition(0, focus_y) //
             | ftxui::frame                     //
             | ftxui::vscroll_indicator         //
             | ftxui::reflect(box_);
    }

    int LogView::viewport_height() const
    {
        if (box_.y_max < box_.y_min)
            return 0;
        // 与 frame 的 external_dimy 口径一致（不含 +1）
        return box_.y_max - box_.y_min;
    }

    int LogView::max_scroll() const
    {
        const int total = static_cast<int>(state_.logs.size());
        return std::max(0, total - viewport_height());
    }

    void LogView::scroll_by(int delta)
    {
        const int max = max_scroll();
        const int old = scroll_;
        scroll_ = std::clamp(scroll_ + delta, 0, max);
        if (scroll_ == old)
            return;

        if (scroll_ >= max)
            auto_follow_ = true; // 滚到底部：恢复跟随
        else
            auto_follow_ = false; // 离开底部：暂停跟随
    }

    void LogView::follow_bottom()
    {
        scroll_ = max_scroll();
    }
} // namespace mud::tui