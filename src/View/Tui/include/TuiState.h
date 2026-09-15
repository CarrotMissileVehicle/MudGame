/**
 * @file TuiState.h
 * @brief FTXUI 界面共享状态。
 *
 * 所有字段仅由主线程读写（FTXUI Loop + PostEventOrExecute 回调均在主线程），
 * 无需加锁。
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace mud::tui
{
    /** @brief 动作类型枚举。 */
    enum class ActionType { None, Fish };

    /** @brief 钓鱼动作的运行时参数（非阻塞定时产出）。 */
    struct FishActionParam
    {
    };

    /** @brief TUI 界面共享状态。 */
    struct TuiState
    {
        // ---- 日志 ----
        std::deque<std::string> logs;
        std::size_t maxLogs = 2000;
        std::size_t totalLogsAppended = 0; // 累计追加条数（单调递增，裁剪不影响）
        std::size_t logsEvicted = 0;       // 累计裁剪条数（队首出队数）

        // ---- 问题提示行（输入框上方）----
        std::string question;

        // ---- 动态完成列表（输入框上方，有输入时显示）----
        std::vector<std::string> completions;

        // ---- 输入框内容 ----
        std::string input;

        // ---- 非阻塞动作状态（当前仅钓鱼）----
        ActionType action_type = ActionType::None;
        std::int64_t action_next_ms = 0;   // 下次产出的毫秒时间戳
        std::size_t action_cycles = 0;     // 已产出次数

        // ---- 输入法提示（idle 时显示的可用指令列表）----
        std::string input_hint;

        /** @brief 向日志追加一行；超过 maxLogs 时裁剪最旧行。 */
        void push_log(std::string line)
        {
            logs.push_back(std::move(line));
            ++totalLogsAppended;
            while (logs.size() > maxLogs)
            {
                logs.pop_front();
                ++logsEvicted;
            }
        }
    };
} // namespace mud::tui
