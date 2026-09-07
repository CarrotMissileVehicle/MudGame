/**
 * @file precondition_test.cpp
 * @brief 采矿前置条件测试（方案 MIN-PR）：等级、照明、层越界等进入矿区的硬门槛。
 *
 * 被测对象：MiningController::start_mining 的前置校验（can_enter_layer / check_lighting）。
 * 数据：mining_layers.json —— 0 shallow(等级1,无照明) / 1 middle(等级3,火把) /
 *       2 deep(等级5,火把) / 3 crystal(等级8,灯笼) / 4 core(等级15,灯笼)。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

using namespace mud;

namespace
{

struct PreFixture
{
    mud::TimeService ts;
    Ore::OreData dummy_ore;
    MiningController controller;
    MiningHandler handler;
    mining::MiningContext ctx;

    PreFixture() : controller(dummy_ore, ts), handler(controller) {}

    // 构造一个同时拥有等级、火把、灯笼的上下文（解锁全部层）。
    mining::MiningContext full()
    {
        mining::MiningContext c;
        c.mining_level = 15;
        c.has_torch = true;
        c.has_lantern = true;
        return c;
    }
};

} // namespace

// MIN-PR-009 采矿等级不足：core(需15) 玩家仅 1 → 拒绝，且无状态副作用
TEST(MiningPrecondition, LevelInsufficientBlocked)
{
    PreFixture f;
    mining::MiningContext low;
    low.mining_level = 1; // 无火把/灯笼 + 等级不足
    EXPECT_FALSE(f.handler.start(4, low)); // core 需等级 15
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_FALSE(f.handler.layer_id().has_value());
}

// MIN-PR-010 照明不足：middle(火把) 无火把拒绝，有火把放行
TEST(MiningPrecondition, LightingTorchRequired)
{
    PreFixture f;
    mining::MiningContext no_light = f.full();
    no_light.has_torch = false;
    no_light.has_lantern = false;
    EXPECT_FALSE(f.handler.start(1, no_light)); // middle 需火把
    EXPECT_FALSE(f.handler.is_mining());

    mining::MiningContext torch_only = f.full();
    torch_only.has_torch = true;
    torch_only.has_lantern = false;
    EXPECT_TRUE(f.handler.start(1, torch_only)); // middle 仅需火把
}

// MIN-PR-010b 照明不足：crystal(灯笼) 有火把无灯笼拒绝，有灯笼放行
TEST(MiningPrecondition, LightingLanternRequired)
{
    PreFixture f;
    mining::MiningContext torch_only = f.full();
    torch_only.has_torch = true;
    torch_only.has_lantern = false;
    EXPECT_FALSE(f.handler.start(3, torch_only)); // crystal 需灯笼

    EXPECT_TRUE(f.handler.start(3, f.full()));
}

// MIN-PR-008 玩家离线 / 空会话上下文：未 start 前 poll/stop 均无副作用（前置空会话兜底）
TEST(MiningPrecondition, IdleSessionHasNoSideEffects)
{
    PreFixture f;
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_TRUE(f.handler.poll(f.ctx).empty());
    EXPECT_TRUE(f.handler.stop(f.ctx).empty());
}

// ---- 未落地能力的前置契约（储量/耐久/背包为数据层字段，当前实现不校验） ----

// MIN-PR-001 无矿镐启动：工具槽为空应拦截（未实现，标注跳过）
TEST(MiningPrecondition, NoToolBlockedNotImplemented)
{
    GTEST_SKIP() << "矿镐存在性前置尚未在字段层实现（方案 MIN-PR-001）";
}

// MIN-PR-002 矿镐耐久为 0 应拦截（未实现，标注跳过）
TEST(MiningPrecondition, ZeroDurabilityBlockedNotImplemented)
{
    GTEST_SKIP() << "矿镐耐久字段与损坏态尚未实现（方案 MIN-PR-002）";
}

// MIN-PR-004 背包满不应启动（未实现，标注跳过）
TEST(MiningPrecondition, BagFullBlockedNotImplemented)
{
    GTEST_SKIP() << "背包容量前置尚未实现（方案 MIN-PR-004）";
}

// MIN-PR-006 矿脉储量 0 不应启动（未实现，标注跳过）
TEST(MiningPrecondition, EmptyReserveBlockedNotImplemented)
{
    GTEST_SKIP() << "矿脉储量字段与采空拦截尚未实现（方案 MIN-PR-006）";
}