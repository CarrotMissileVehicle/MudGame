/**
 * @file load_test.cpp
 * @brief 高并发/负载/瞬时洪峰测试（tests/timeservice/load_concurrency/ 用例 TS-LC-*）。
 *
 * 验证大批量定时注册、单帧到期洪峰、洪峰期间取消子集、批量取消无泄漏、
 * 回调内大批量注册等压力场景下的正确性与稳定性。
 */
#include "time_service.h"

#include <gtest/gtest.h>

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

TEST(Load, BulkRegisterThenSingleFrameFlood) // TS-LC-001/002 万级同刻洪峰
{
    mud::TimeService svc; // 00:00
    constexpr int N = 10000;
    int fired = 0;
    for (int i = 0; i < N; ++i)
        svc.schedule_time(dt(0, 1, 1, 0, 1), [&] { ++fired; });
    push(svc, 1); // 跨过 00:01
    EXPECT_EQ(fired, N); // 全部触发且各一次
    push(svc, 5);
    EXPECT_EQ(fired, N); // 不重复
}

TEST(Load, FloodWithPartialCancel) // TS-LC-003 洪峰期间取消子集
{
    mud::TimeService svc;
    constexpr int N = 10000;
    constexpr int K = 3000;
    int fired = 0;
    std::vector<std::size_t> toks;
    toks.reserve(N);
    for (int i = 0; i < N; ++i)
        toks.push_back(svc.schedule_time(dt(0, 1, 1, 0, 1), [&] { ++fired; }));
    for (int i = 0; i < K; i += 7) svc.cancel(toks[i]); // 取消部分
    push(svc, 1);
    EXPECT_EQ(fired, N - ((K + 6) / 7));
}

TEST(Load, BulkIntervalThenCancelAll) // TS-LC-004 批量周期全部取消
{
    mud::TimeService svc;
    constexpr int N = 5000;
    std::int64_t fired = 0;
    std::vector<std::size_t> toks;
    toks.reserve(N);
    for (int i = 0; i < N; ++i)
        toks.push_back(svc.schedule_interval(1, [&] { ++fired; }));
    for (auto t : toks) svc.cancel(t);
    push(svc, 10);
    EXPECT_EQ(fired, 0);
}

TEST(Load, RegisterDuringFloodCallback) // TS-LC-006 回调内批量注册
{
    mud::TimeService svc; // 00:00
    std::int64_t first = 0, second = 0;
    svc.schedule_time(dt(0, 1, 1, 0, 2), [&] {
        ++first;
        for (int i = 0; i < 100; ++i)
            svc.schedule_time(dt(0, 1, 1, 0, 4), [&] { ++second; });
    });
    push(svc, 5);
    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 100); // 回调内注册的 100 条在后续到期全部触发
}

TEST(Load, StableOrderUnderManyDue) // TS-LC-002b 大批量到期顺序稳定
{
    mud::TimeService svc;
    constexpr int N = 2000;
    std::vector<int> seq;
    seq.reserve(N);
    // 递增 due，注册打乱顺序
    for (int i = N; i >= 1; --i)
        svc.schedule_time(dt(0, 1, 1, 0, i), [&, i] { seq.push_back(i); });
    push(svc, N);
    ASSERT_EQ(seq.size(), std::size_t(N));
    for (int i = 0; i < N; ++i) EXPECT_EQ(seq[i], i + 1); // 按 due 升序
}