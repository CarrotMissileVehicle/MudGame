/**
 * @file time_service.h
 * @brief 游戏时间服务（TimeService）：游戏帧驱动的时间推进与定时回调调度。
 *
 * 时间由外部游戏循环通过 update(real_delta) 显式推进，时间仍映射真实时间
 * （真实经过时长 × time_scale）。移除旧的事件推送（TimeEvent 枚举），
 * 改为一次性/周期定时回调函数数组，满足同一帧内跨多个游戏时刻的顺序触发。
 */
#pragma once

#include "game_time.h"

#include <cstddef>
#include <functional>
#include <vector>

namespace mud
{
    /** @brief 游戏时间服务：帧驱动推进 + 定时回调调度器。 */
    class TimeService
    {
    public:
        /** @brief 定时回调类型（无参数，到点即触发）。 */
        using Callback = std::function<void()>;

        /** @brief 游戏日历字段，供每帧轮询方自行判断分钟/时/日/月/年边界。 */
        struct CalendarFields
        {
            int year;
            unsigned month;
            unsigned day;
            int hour;
            int minute;
        };

    public:
        /// @param origin 游戏世界纪元偏移（0 时刻，默认 0）。
        /// @param total_runtime 初始累计游戏时长（来自持久化，默认 0）。
        explicit TimeService(time::gameTimePoint origin = time::gameTimePoint{},
                             time::gameDuration total_runtime = time::gameDuration{0});

        /** @brief 返回当前游戏时间点（连续）。 */
        [[nodiscard]] time::gameTimePoint now() const;

        /** @brief 返回当前累计总长（供调用方持久化）。 */
        [[nodiscard]] time::gameDuration session_total() const;

        /** @brief 每帧推进游戏时间；real_delta 为真实经过时长，受 time_scale_ 影响。 */
        void update(time::gameDuration real_delta);

        /** @brief 直接设置当前游戏时间。 */
        void set_time(time::gameTimePoint time);

        /** @brief 设置时间流速倍率。 */
        void set_time_scale(double scale);

        /** @brief 返回当前时间流速倍率。 */
        [[nodiscard]] double time_scale() const;

        /** @brief 返回当前游戏日历字段（供轮询判断分/时/日/月/年边界）。 */
        [[nodiscard]] CalendarFields calendar_fields() const;

        /** @brief 注册一次性定时回调：到达 due 时刻触发后移除。返回退订令牌。 */
        std::size_t schedule_time(time::gameTimePoint due, Callback callback);

        /** @brief 注册周期定时回调：每 period 触发一次并自动重排。返回退订令牌。 */
        std::size_t schedule_interval(time::gameDuration period, Callback callback);

        /** @brief 退订指定令牌对应的定时回调。 */
        void cancel(std::size_t token);

    private:
        // 定时回调条目：退订令牌 + 到期时刻 + 周期(0=一次性) + 回调
        struct Entry
        {
            std::size_t token;
            time::gameTimePoint due;
            time::gameDuration period{0};
            Callback callback;
        };

        time::gameTimePoint startTime_;       // 游戏绝对时间零点（0 时刻）
        time::gameDuration accumulated_{0};   // 累计游戏时长（唯一推进来源）
        double time_scale_{60.0};             // 时间流速倍率
        std::vector<Entry> schedule_;         // 定时回调函数数组
        std::size_t next_token_{1};           // 下一个分配令牌
    };
}