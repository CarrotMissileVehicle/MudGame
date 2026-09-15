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
    /** @brief 总分钟数上界：32767-12-31 23:59 距纪元的总分钟数（DEF-402）。 */
    inline constexpr std::int64_t kMaxTotalMinutes = [] {
        const auto days =
            std::chrono::sys_days{std::chrono::year{32767} / 12 / 31} -
            std::chrono::sys_days{std::chrono::year{0} / 1 / 1};
        return days.count() * 1440LL + 23 * 60 + 59;
    }();

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

        /** @brief 校验日历字段是否合法（月 1-12、日与年月匹配、时 0-23、分 0-59）。 */
        [[nodiscard]] bool valid() const noexcept
        {
            if (hour > 23 || minute > 59)
                return false;
            const std::chrono::year_month_day ymd{
                std::chrono::year{year},
                std::chrono::month{month},
                std::chrono::day{day}};
            return ymd.ok();
        }

        /** @brief 返回距纪元（0年1月1日 00:00）的总分钟数，供持久化与比较。 */
        [[nodiscard]] std::int64_t total_minutes() const
        {
            // 前置条件：字段合法。非法字段（month>12、day>31、平年 2/29 等）转换
            // ymd 为 sys_days 是未定义行为，一律按纪元起点（0 分钟）夹紧处理，
            // 保证任何输入（含存档反序列化）都不会进入 UB。
            if (!valid())
                return 0;
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
            const std::int64_t cur = total_minutes();
            // DEF-402：饱和运算防有符号溢出。minutes 可来自外部（如存档
            // gameTotalMinutes 经 advance() 重建时钟），极大正/负值会让
            // cur + minutes 溢出为 UB。上界 = std::chrono::year 合法区间
            // 上限（32767 年 12 月 31 日 23:59）对应的总分钟数，超过后
            // ymd.year() 写回 year 会使下一次 total_minutes() 进入非法输入。
            std::int64_t total;
            if (minutes > 0 && cur > kMaxTotalMinutes - minutes)
                total = kMaxTotalMinutes;
            else if (minutes < 0 && minutes < -cur)
                total = 0;
            else
                total = cur + minutes;
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