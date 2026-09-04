/**
 * @file calendar_test.cpp
 * @brief GameDateTime 日历字段测试（tests/timeservice/calendar_event/ 用例 TS-CE-*）。
 *
 * 验证跨月/跨年/闰年进位、total_minutes() 距纪元换算及其双向互逆、纪元负值夹紧。
 * 以独立预期（2560 天/1440 分钟等字面量）为判据，不依赖被测代码自身结果。
 */
#include "game_time.h"

#include <gtest/gtest.h>

namespace
{
using mud::time::GameDateTime;

// 构造辅助：形参顺序与结构体字段一致（年/月/日/时/分）。
GameDateTime dt(int y, unsigned m, unsigned d, unsigned h = 0, unsigned min = 0)
{
    return GameDateTime{y, m, d, h, min};
}

TEST(Calendar, EpochIsZeroMinutes) // TS-CE-001 纪元基准
{
    const auto t = dt(0, 1, 1, 0, 0);
    EXPECT_EQ(t.total_minutes(), 0);
}

TEST(Calendar, MinuteOfDay) // 时分换算
{
    EXPECT_EQ(dt(0, 1, 1, 1, 30).total_minutes(), 90);   // 01:30
    EXPECT_EQ(dt(0, 1, 1, 0, 1).total_minutes(), 1);     // 00:01
    EXPECT_EQ(dt(0, 1, 1, 23, 59).total_minutes(), 1439); // 23:59
}

TEST(Calendar, DayOfMonthIs1440) // 月内天=1440 分钟
{
    EXPECT_EQ(dt(0, 1, 2, 0, 0).total_minutes(), 1440);
}

TEST(Calendar, CrossMonthCarry) // TS-CE-001 跨月进位：1月31日 23:59→2月1日 00:00
{
    auto t = dt(0, 1, 31, 23, 59);
    t.advance(1);
    EXPECT_EQ(t, dt(0, 2, 1, 0, 0));
}

TEST(Calendar, CrossYearCarry) // TS-CE-002 跨年进位：0年12月31日→1年1月1日
{
    auto t = dt(0, 12, 31, 23, 59);
    t.advance(1);
    EXPECT_EQ(t, dt(1, 1, 1, 0, 0));
}

TEST(Calendar, LeapFebruary) // TS-CE-003 闰年：2月28日 23:59→2月29日 00:00
{
    auto t = dt(4, 2, 28, 23, 59);
    t.advance(1);
    EXPECT_EQ(t, dt(4, 2, 29, 0, 0));
}

TEST(Calendar, NonLeapFebruary) // TS-CE-004 平年：2月28日 23:59→3月1日 00:00
{
    auto t = dt(3, 2, 28, 23, 59);
    t.advance(1);
    EXPECT_EQ(t, dt(3, 3, 1, 0, 0));
}

TEST(Calendar, LeapYearDaysInFebruary) // 闰/平年 2 月天数，经 total_minutes 差分独立校验
{
    // 闰年（year 4）：2/1 → 3/1 差 29 天
    EXPECT_EQ(dt(4, 3, 1).total_minutes() - dt(4, 2, 1).total_minutes(), 29 * 1440);
    // 平年（year 3）：差 28 天
    EXPECT_EQ(dt(3, 3, 1).total_minutes() - dt(3, 2, 1).total_minutes(), 28 * 1440);
    // 纪元年 year 0 为闰年（可被 400 整除）：0/1/1 → 1/1/1 差 366 天
    EXPECT_EQ(dt(1, 1, 1).total_minutes() - dt(0, 1, 1).total_minutes(), 366 * 1440);
    // 平年（year 1）：1/1/1 → 2/1/1 差 365 天
    EXPECT_EQ(dt(2, 1, 1).total_minutes() - dt(1, 1, 1).total_minutes(), 365 * 1440);
}

TEST(Calendar, LargeSpanCrossesMultipleMonths) // TS-CE-007 大跨度推进的分段一致性
{
    auto step     = dt(1, 1, 1);
    auto leap     = dt(2, 3, 1); // year1 non-leap: Jan+Feb=59 天，再从 epoch 直接取
    const auto big = dt(2, 3, 1);
    step.advance(366 * 1440 + 59 * 1440); // 从 epoch 推到 2/3/1
    EXPECT_EQ(step, big);
    EXPECT_EQ(step.total_minutes(), big.total_minutes());
}

TEST(Calendar, AdvanceBackwardIsInverse) // TS-CE-005 total_minutes 双向互逆
{
    const auto start = dt(5, 10, 20, 8, 45);
    const auto min   = start.total_minutes();
    auto fw          = start;
    fw.advance(+1440); // +1 天
    EXPECT_EQ(fw.total_minutes(), min + 1440);
    fw.advance(-1440); // 回推
    EXPECT_EQ(fw.total_minutes(), min);
    EXPECT_EQ(fw, start);
}

TEST(Calendar, NegativeAdvanceClampedToEpoch) // TS-CE-006 纪元夹紧，不回退到负
{
    auto t = dt(0, 1, 1, 0, 0);
    t.advance(-5);
    EXPECT_EQ(t, dt(0, 1, 1, 0, 0));
    EXPECT_EQ(t.total_minutes(), 0);
}

TEST(Calendar, ComparisonOrdersChronologically) // 字典序比较即时间顺序
{
    EXPECT_LT(dt(1, 12, 31, 23, 59), dt(2, 1, 1, 0, 0));
    EXPECT_LT(dt(0, 1, 1, 0, 0), dt(0, 1, 1, 0, 1));
}
} // namespace