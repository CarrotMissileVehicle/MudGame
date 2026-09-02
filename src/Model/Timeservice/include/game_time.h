/**
 * @file game_time.h
 * @brief 游戏时间基础类型定义。
 *
 * 游戏时间采用显式日历字段结构体（Year/Month/Day/Hour/Minute），
 * 最小粒度为分钟、不含秒。以 0 年 1 月 1 日 00:00 为纪元。
 * 日期进位与天差换算复用 std::chrono 的日历能力，保证闰年与月天数正确。
 */
#pragma once

#include <chrono>
#include <compare>
#include <cstdint>

namespace mud::time
{
    /** @brief 游戏时间：显式日历字段，最小粒度=分钟，纪元=0年1月1日 00:00。 */
    struct GameDateTime
    {
        int year{0};        // 年（0 起）
        unsigned month{1};  // 月（1-12）
        unsigned day{1};    // 日（1-31）
        unsigned hour{0};   // 时（0-23）
        unsigned minute{0}; // 分（0-59）

        /** @brief 默认字典序比较即时间顺序（年→月→日→时→分）。 */
        auto operator<=>(const GameDateTime&) const = default;

        /** @brief 返回距纪元（0年1月1日 00:00）的总分钟数，供持久化与比较。 */
        [[nodiscard]] std::int64_t total_minutes() const
        {
            const std::chrono::year_month_day ymd{
                std::chrono::year{year},
                std::chrono::month{month},
                std::chrono::day{day}};
            const auto days =
                std::chrono::sys_days{ymd} - std::chrono::sys_days{std::chrono::year{0} / 1 / 1};
            return days.count() * 1440 + static_cast<std::int64_t>(hour) * 60 + minute;
        }

        /** @brief 向后推进 minutes 分钟（可为负），处理分→时→日→月→年进位。 */
        void advance(std::int64_t minutes)
        {
            std::int64_t total = total_minutes() + minutes;
            if (total < 0) total = 0; // 不得早于纪元
            const std::int64_t days = total / 1440;
            const std::int64_t rem = total % 1440;
            const auto ymd = std::chrono::year_month_day{
                std::chrono::sys_days{std::chrono::year{0} / 1 / 1} + std::chrono::days{days}};
            year = static_cast<int>(ymd.year());
            month = static_cast<unsigned>(ymd.month());
            day = static_cast<unsigned>(ymd.day());
            hour = static_cast<unsigned>(rem / 60);
            minute = static_cast<unsigned>(rem % 60);
        }
    };

    /** @brief 时长单位：游戏分钟。 */
    using gameMinutes = std::int64_t;
}