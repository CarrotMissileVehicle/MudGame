/**
 * @file LogView.h
 * @brief 可滚动的日志视图组件（基于 FTXUI ComponentBase）。
 *
 * 功能：
 *  - 鼠标滚轮在日志区域上/下滑动查看历史日志；
 *  - 默认“跟随底部”：新日志到达时自动滚到最下方；
 *  - 用户手动向上滚动后暂停跟随，滚回底部时恢复；
 *  - 新输入（提交命令）时调用 force_follow() 强制回到底部。
 *
 * 不抢占输入框焦点：滚轮事件仅在光标位于日志区域内时被处理。
 */
#pragma once

#include <cstddef>

#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/box.hpp>

namespace mud::tui
{
    struct TuiState;

    /** @brief 显示 TuiState::logs 的可滚动日志组件。 */
    class LogView : public ftxui::ComponentBase
    {
    public:
        explicit LogView(TuiState& state);

        /** @brief 恢复跟随底部并立即滚到最下方（提交新命令时调用）。 */
        void force_follow();

        ftxui::Element OnRender() override;

    private:
        bool OnEvent(ftxui::Event event) override;

        int viewport_height() const;
        int max_scroll() const;
        void scroll_by(int delta);
        void follow_bottom();

        TuiState& state_;
        ftxui::Box box_;            // 上一帧日志视口区域（由 reflect 记录）
        int scroll_ = 0;            // 当前滚动偏移（行，0 = 最顶部）
        bool auto_follow_ = true;   // 是否跟随底部
        std::size_t seen_appended_ = 0; // 上次渲染时已处理的累计追加条数
        std::size_t seen_evicted_ = 0;  // 上次渲染时已处理的累计裁剪条数
    };
} // namespace mud::tui