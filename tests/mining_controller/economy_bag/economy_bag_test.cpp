/**
 * @file economy_bag_test.cpp
 * @brief 经济与背包结算契约测试（方案 MIN-EC）。
 *
 * 目标不变量：产出正确落袋、耐久逐次损耗且不为负、包满/负重有明确终态、经验可累计。
 * 现状：背包容器、负重、耐久损耗字段为未落地能力，用例以 GTEST_SKIP 锁定契约。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

using namespace mud;

namespace
{
struct EC { Ore::OreData ore; }; // 契约占位
} // namespace

// MIN-EC-001 背包有格时产出正确落袋
TEST(MiningEconomy, DepositToBagOnSlot)
{
    GTEST_SKIP() << "背包容器未实现（方案 MIN-EC-001）：断言矿石+1、经验累加";
}

// MIN-EC-002 背包满时产出走待领/邮件或明确丢弃，不静默吞没
TEST(MiningEconomy, BagFullRouteToMail)
{
    GTEST_SKIP() << "背包容量未实现（方案 MIN-EC-002）：断言包满结算有明确终态";
}

// MIN-EC-004 矿镐耐久逐次损耗且不为负
TEST(MiningEconomy, DurabilityConsumesPerPoll)
{
    GTEST_SKIP() << "矿镐耐久字段未实现（方案 MIN-EC-004）：断言耐久=次数差、归0不再产出";
}

// MIN-EC-007 经验结算可累计、不重复
TEST(MiningEconomy, ExperienceAccumulates)
{
    GTEST_SKIP() << "经验累计接口未实现（方案 MIN-EC-007）：断言经验只增不回退";
}