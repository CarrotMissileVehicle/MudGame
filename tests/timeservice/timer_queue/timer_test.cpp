/**
 * @file timer_test.cpp
 * @brief 定时器队列测试（tests/timeservice/timer_queue/ 用例 TS-TQ-*）。
 *
 * 验证一次性/周期定时到期触发、同帧多到期顺序、回调内再注册、令牌唯一、
 * 过去时刻到期不触发等行为。
 */
#include "time_service.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace
{
using mud::time::GameDateTime;

GameDateTime dt(int y, unsigned m, unsigned d, unsigned h = 0, unsigned min = 0)
{
    return GameDateTime{y, m, d, h, min};
}

void push(mud::TimeService& svc, std::int64_t frames)
{
    for (std::int64_t i = 0; i < frames; ++i) svc.update();
}
} // namespace

TEST(Timer, OneShotFiresOnceThenRemoved) // TS-TQ-001
{
    mud::TimeService svc;
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 5), [&] { ++fired; });
    push(svc, 5);
    EXPECT_EQ(fired, 1);
    push(svc, 30); // 越过后不再触发（已移除）
    EXPECT_EQ(fired, 1);
}

TEST(Timer, PastDueDoesNotFire) // TS-TQ-002/011 过去时刻到期不触发
{
    mud::TimeService svc(dt(0, 1, 1, 0, 10)); // 初始 00:10
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 5), [&] { ++fired; }); // due 早于 now
    push(svc, 3);
    EXPECT_EQ(fired, 0);
}

TEST(Timer, FiresAtWindowRightEdge) // TS-TQ-003 (before, after] 右闭
{
    mud::TimeService svc;
    int fired = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 1), [&] { ++fired; });
    push(svc, 1); // 推进后 now=00:01，due==after 触发
    EXPECT_EQ(fired, 1);
}

TEST(Timer, MultipleDueFiredInOrder) // TS-TQ-005 同帧多到期顺序 003→005→010
{
    mud::TimeService svc;
    std::vector<int> seq;
    svc.schedule_time(dt(0, 1, 1, 0, 10), [&] { seq.push_back(10); });
    svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { seq.push_back(3); });
    svc.schedule_time(dt(0, 1, 1, 0, 5), [&] { seq.push_back(5); });
    push(svc, 10);
    EXPECT_EQ(seq, (std::vector<int>{3, 5, 10}));
}

TEST(Timer, SameDueFiredInTokenOrder) // TS-TQ-005b 同位次按 token 升序
{
    mud::TimeService svc;
    std::vector<int> seq;
    const auto t1 = svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { seq.push_back(100); });
    const auto t2 = svc.schedule_time(dt(0, 1, 1, 0, 3), [&] { seq.push_back(200); });
    (void)t1;
    (void)t2;
    push(svc, 5);
    EXPECT_EQ(seq, (std::vector<int>{100, 200}));
}

TEST(Timer, IntervalReschedulesPeriodically) // TS-TQ-006 周期续排
{
    mud::TimeService svc; // 00:00
    int fired = 0;
    svc.schedule_interval(5, [&] { ++fired; }); // 首次 00:05
    push(svc, 5);
    EXPECT_EQ(fired, 1);
    push(svc, 5); // 二期 00:10
    EXPECT_EQ(fired, 2);
    push(svc, 5); // 三期 00:15
    EXPECT_EQ(fired, 3);
}

TEST(Timer, RegisterDuringCallbackIsSafe) // TS-TQ-009 回调内再注册不迭代失效
{
    mud::TimeService svc; // 00:00
    int a = 0, b = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 3), [&] {
        ++a;
        svc.schedule_time(dt(0, 1, 1, 0, 6), [&] { ++b; }); // 回调内注册 B
    });
    push(svc, 10);
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, 1); // B 在 00:06 到期触发
}

TEST(Timer, TokensAreUniqueIncreasing) // TS-TQ-012 令牌唯一递增
{
    mud::TimeService svc;
    std::vector<std::size_t> tokens;
    for (int i = 0; i < 100; ++i)
        tokens.push_back(svc.schedule_interval(1, [] {}));
    for (std::size_t i = 1; i < tokens.size(); ++i)
    {
        EXPECT_NE(tokens[i], tokens[i - 1]);
        EXPECT_GT(tokens[i], tokens[i - 1]);
    }
}

TEST(Timer, ManyFramesSpansMultipleDueWindows) // 长跑跨多到期窗口稳定
{
    mud::TimeService svc;
    int fired = 0;
    svc.schedule_interval(1440, [&] { ++fired; }); // 每日
    push(svc, 1440 * 3); // 3 天等效
    EXPECT_EQ(fired, 3);
}