/**
 * @file time_event.h
 * @brief 游戏时间事件枚举定义。
 *
 * 定义时间推进到不同粒度（分/时/日/月/年）时触发的通知事件类型。
 */
#pragma once

namespace mud::time
{

    /** @brief 时间粒度过界事件类型。 */
    enum class TimeEvent
    {
        MinuteChanged,  // 分钟变更
        HourChanged,    // 小时变更
        DayChanged,     // 跨天
        MonthChanged,   // 跨月
        YearChanged     // 跨年
    };

}