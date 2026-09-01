/**
 * @file time_service.h
 * @brief 游戏时间服务（TimeService）。
 *
 * 管理游戏当前时间、时间流速倍率，并支持按真实间隔推进游戏时间、
 * 设置时间、订阅时间事件。
 *
 * 依赖：game_time、time_event。
 */
#pragma once

#include "game_time.h"
#include "time_event.h"

#include <functional>
#include <vector>

namespace mud
{

    /** @brief 游戏时间服务：统一驱动游戏内时间推进与事件通知。 */
    class TimeService
    {
    private:
        time::gameTimePoint startTime_;     // 游戏绝对时间零点（0 时刻）
        time::gameTimePoint session_start_; // 本次真实会话起点（构造时注入）
        time::gameTimePoint last_tick_time_; // 上一帧时间戳
        time::gameDuration total_runtime_;  // 历史累计游戏运行总长（来自 JSON）

    public:
        /** @brief 时间事件监听器回调类型。 */
        using Listener = std::function<void(time::TimeEvent)>;

    public:
        explicit TimeService(time::gameTimePoint origin,
                             time::gameDuration total_runtime,
                             time::gameTimePoint session_start);

        /** @brief 返回当前游戏时间点。 */
        [[nodiscard]] time::gameTimePoint now() const;

        /** @brief 返回含本次会话的当前累计总长（供调用方持久化）。 */
        [[nodiscard]] time::gameDuration session_total() const;

        /** @brief 按真实经过时长推进游戏时间（受时间倍率影响）。 */
        void tick(time::gameDuration real_delta);

        /** @brief 直接设置当前游戏时间。 */
        void set_time(time::gameTimePoint time);

        /** @brief 设置游戏时间流速倍率。 */
        void set_time_scale(double scale);

        /** @brief 返回当前时间流速倍率。 */
        [[nodiscard]] double time_scale() const;

        /** @brief 注册时间事件监听器。 */
        void subscribe(Listener listener);

    private:
        double time_scale_{1.0};          // 时间流速倍率（默认 1.0）
        std::vector<Listener> listeners_; // 时间事件订阅者

        // 本次会话已按倍率折算的推进时长
        [[nodiscard]] time::gameDuration live_elapsed_scaled() const;
    };

}