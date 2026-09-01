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

namespace mud
{

    /** @brief 游戏时间服务：统一驱动游戏内时间推进与事件通知。 */
    class TimeService
    {
    public:
        /** @brief 时间事件监听器回调类型。 */
        using Listener = std::function<void(time::TimeEvent)>;

    public:
        TimeService();

        /** @brief 返回当前游戏时间点。 */
        time::gameTimePoint now() const;

        /** @brief 按真实经过时长推进游戏时间（受时间倍率影响）。 */
        void tick(time::gameDuration real_delta);

        /** @brief 直接设置当前游戏时间。 */
        void set_time(time::gameTimePoint time);

        /** @brief 设置游戏时间流速倍率。 */
        void set_time_scale(double scale);

        /** @brief 返回当前时间流速倍率。 */
        double time_scale() const;

        /** @brief 注册时间事件监听器。 */
        void subscribe(Listener listener);

    private:
        time::gameTimePoint current_time_; // 当前游戏时间
        double time_scale_{1.0};           // 时间流速倍率（默认 1.0）

        // 具体实现暂时隐藏
    };

}