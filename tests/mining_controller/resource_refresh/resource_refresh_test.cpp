/**
 * @file resource_refresh_test.cpp
 * @brief 资源刷新契约测试（方案 MIN-RF）。
 *
 * 目标不变量：采空后提示、按刷新周期重置、重复刷新幂等、重启持久化不破不变量。
 * 现状：矿脉储量与刷新调度为未落地能力，用例以 GTEST_SKIP 锁定契约。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

using namespace mud;

namespace
{
struct RF { Ore::OreData ore; }; // 契约占位
} // namespace

// MIN-RF-001 采空后明确提示且访问被拒
TEST(MiningRefresh, EmptyDepositHint)
{
    GTEST_SKIP() << "矿脉储量未实现（方案 MIN-RF-001）：断言采空提示与拒绝访问";
}

// MIN-RF-002 按周期触发后储量恢复满值
TEST(MiningRefresh, RefreshRestoresFullReserve)
{
    GTEST_SKIP() << "刷新调度未实现（方案 MIN-RF-002）：断言刷新后储量=配置满值";
}

// MIN-RF-004 重复刷新（未到周期）幂等，不加量
TEST(MiningRefresh, DuplicateRefreshIdempotent)
{
    GTEST_SKIP() << "刷新幂等未实现（方案 MIN-RF-004）：断言重复刷新不重复加量";
}

// MIN-RF-007 重启后刷新状态持久化，采空脉不因重启被重置
TEST(MiningRefresh, RestartPersistsState)
{
    GTEST_SKIP() << "持久化未实现（方案 MIN-RF-007）：断言重启不破坏刷新不变量";
}