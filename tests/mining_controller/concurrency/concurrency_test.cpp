/**
 * @file concurrency_test.cpp
 * @brief 并发抢矿契约测试（方案 MIN-CC）。
 *
 * 目标不变量：同一矿脉「总产出 ≤ 初始储量」；储量剩 1 时竞争恰好 1 人获胜；
 * 绝无负库存、无重复扣减、无超采。
 * 现状：矿脉储量字段与锁机制为未落地能力，用例以 GTEST_SKIP 锁定契约，待落地后启用。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

using namespace mud;

namespace
{
struct CC { Ore::OreData ore; }; // 契约占位：无被测实现
} // namespace

// MIN-CC-003 储量剩 1 多线程竞争：恰 1 人成功
TEST(MiningConcurrency, ExclusiveWinnerWhenReserveOne)
{
    GTEST_SKIP() << "矿脉储量与锁机制未实现（方案 MIN-CC-003）：断言恰 1 成功、总产出=1、无负库存";
}

// MIN-CC-004 储量 0 后全部拒绝
TEST(MiningConcurrency, DepletedRejectsAll)
{
    GTEST_SKIP() << "矿脉储量未实现（方案 MIN-CC-004）：断言 reserve 0 后所有结算被拒";
}

// MIN-CC-005 同会话重复结算经幂等去重只生效一次
TEST(MiningConcurrency, SettlementIdempotent)
{
    GTEST_SKIP() << "结算幂等键未实现（方案 MIN-CC-005）：断言重复 poll 不重复产出";
}

// MIN-CC-001 储量扣减原子：并发总产出 ≤ 初始储量
TEST(MiningConcurrency, AtomicReserveDecrement)
{
    GTEST_SKIP() << "原子储量扣减未实现（方案 MIN-CC-001）：断言总产出≤初始储量";
}