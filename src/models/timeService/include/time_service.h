/**
 * @file time_service.h
 * @brief 游戏时间服务（TimeService）：游戏帧驱动的时间推进与定时回调调度。
 *
 * 时间由外部游戏循环通过 update() 显式推进（每帧推进 1 现实秒）。
 * 游戏时间以显式日历字段（GameDateTime）存储，最小粒度为分钟、不含秒。
 * 时间流速倍率（time_scale，游戏秒/现实秒）可变；非整分钟的倍率由
 * 亚分钟余数吸收，不引入秒级显示。
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

        /// @param initial 初始游戏时间（纪元起默认 0年1月1日 00:00）。
        explicit TimeService(time::GameDateTime initial = time::GameDateTime{});

        /** @brief 返回当前游戏时间（显式日历，无秒）。 */
        [[nodiscard]] time::GameDateTime now() const;

        /** @brief 返回距纪元的总分钟数（供调用方持久化）。 */
        [[nodiscard]] std::int64_t session_total() const;

        /** @brief 推进一帧 = 1 现实秒（受 time_scale 影响）。 */
        void update();

        /** @brief 直接设置当前游戏时间。 */
        void set_time(time::GameDateTime t);

        /** @brief 设置时间流速倍率（游戏秒/现实秒）。 */
        void set_time_scale(double scale);

        /** @brief 返回当前时间流速倍率。 */
        [[nodiscard]] double time_scale() const;

        /** @brief 注册一次性定时回调：到达 due 时刻触发后移除。返回退订令牌。 */
        std::size_t schedule_time(time::GameDateTime due, Callback callback);

        /** @brief 注册周期定时回调：每 minutes 分钟触发一次并自动重排。 */
        std::size_t schedule_interval(std::int64_t minutes, Callback callback);

        /** @brief 退订指定令牌对应的定时回调。 */
        void cancel(std::size_t token);

    private:
        // 定时回调条目：退订令牌 + 到期时刻 + 周期分钟数(0=一次性) + 回调
        struct Entry
        {
            std::size_t token;
            time::GameDateTime due;
            std::int64_t period{0};
            Callback callback;
        };

        time::GameDateTime now_;     // 唯一时间真相：显式日历，无秒
        double time_scale_{60.0};    // 时间流速倍率（游戏秒/现实秒）
        double sub_minute_{0.0};     // 亚分钟余数（游戏秒，[0,60)）
        std::vector<Entry> schedule_; // 定时回调函数数组
        std::size_t next_token_{1};   // 下一个分配令牌
    };
}