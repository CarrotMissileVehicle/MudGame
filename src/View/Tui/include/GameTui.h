/**
 * @file GameTui.h
 * @brief 基于 FTXUI 的终端图形界面宿主。
 *
 * 布局（从上到下）：
 *   ┌───────────────────────────────────┐
 *   │ 日志区（scrollable，占据全部高度） │
 *   ├───────────────────────────────────┤
 *   │ 动态完成列表（无输入时隐藏）       │
 *   │ 问题提示行（如"请选择作物"）       │
 *   │ > 输入框                         │
 *   └───────────────────────────────────┘
 *
 * 用法：
 *   GameTui tui(state);
 *   tui.set_process_line(fn);     // 设置命令处理回调
 *   tui.set_completions(fn);      // 设置动态补全回调
 *   tui.set_tick(fn);             // 设置定时回调（每 tick 驱动）
 *   tui.run();                    // 阻塞运行 FTXUI 事件循环
 *   // 从后台线程调用 tui.post_background(fn) 以在主线程执行回调
 */
#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "TuiState.h"

namespace ftxui { class App; }

namespace mud::tui
{
    class GameTui
    {
    public:
        explicit GameTui(TuiState& state);
        ~GameTui();

        /** @brief 设置命令处理回调：接收一行输入文本，返回 true 表示已处理。 */
        void set_process_line(std::function<bool(const std::string&)> fn);

        /** @brief 设置动态补全回调：返回当前输入对应的候选列表。 */
        void set_completions(std::function<std::vector<std::string>(const std::string&)> fn);

        /** @brief 设置 tick 回调（每次 FTXUI 事件循环迭代时调用）。 */
        void set_tick(std::function<void()> fn);

        /**
         * @brief 阻塞运行 FTXUI 事件循环（Fullscreen 模式）。
         *
         * 调用后阻塞直到 Exit() 被触发。
         */
        void run();

        /** @brief 请求退出事件循环（线程安全）。 */
        void exit();

        /**
         * @brief 向主线程投递一个回调并在主线程执行（线程安全）。
         *
         * 用于后台线程触发 tick_world 等操作。
         */
        void post_background(std::function<void()> fn);

    private:
        TuiState& state_;
        std::unique_ptr<ftxui::App> app_;

        std::function<bool(const std::string&)> process_line_;
        std::function<std::vector<std::string>(const std::string&)> completions_;
        std::function<void()> tick_;

        struct Impl;
        std::unique_ptr<Impl> impl_;
    };
} // namespace mud::tui
