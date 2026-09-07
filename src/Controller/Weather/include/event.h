/**
 * @file event.h
 * @brief 事件模型（mud::event）。
 *
 * 定义采矿结算时的随机事件判定接口（宝箱翻倍 / 塌方中断）。
 * 每日天气事件（generate_daily_events 等）本次不实现，留白。
 */
#pragma once

namespace mud::event
{
    /** @brief 采矿随机事件系统：依据随机数判定本轮采矿产出的宝箱/塌方事件。 */
    struct EventSystem
    {
        /**
         * @brief 采矿随机事件判定。
         * @param found_chest 命中宝箱（rand_chance >= 93，约 8%）：当次产出翻倍。
         * @param cave_in     命中塌方（rand_chance < 5，约 5%）：中断本次结算。
         * @param rand_chance 均匀落入 [1,100] 的随机数。
         *
         * 区间 1-4 为塌方、93-100 为宝箱，互不重叠。
         */
        void roll_mining_event(bool& found_chest, bool& cave_in, int rand_chance) const;
    };
}