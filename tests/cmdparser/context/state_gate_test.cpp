/**
 * @file state_gate_test.cpp
 * @brief 上下文级测试（方案 CP-C）：采矿会话状态机、硬直、等级/照明权限拦截。
 *
 * 被测对象：MiningController + MiningHandler + MiningState。
 * 数据来自 MUDGAME_DATA_DIR 下的 mining_layers.json（层等级/照明）与
 * spawn_rates.json / ore.json。
 *
 * 层索引：0 shallow(等级1,无照明) / 1 middle(等级3,火把) /
 *         2 deep(等级5,火把) / 3 crystal(等级8,灯笼) / 4 core(等级15,灯笼)
 *
 * 眩晕/死亡对所有指令的"全网关"拦截尚未在命令层实现，以 GTEST_SKIP 标注。
 */
#include <gtest/gtest.h>

#include "mining_handler.h"
#include "mining_state.h"
#include "mining_types.h"
#include "time_service.h"

using namespace mud;

namespace {

// 构造一套可复用的控制器 + 会话处理器。
struct GateFixture
{
    TimeService ts;
    Ore::OreData dummy_ore;          // 构造入参占位（控制器内部自加载真实数据）
    MiningController controller;
    MiningHandler handler;

    // 默认采矿上下文：等级足、有火把与灯笼，便于聚焦状态机。
    mining::MiningContext ok_ctx;

    GateFixture()
        : controller(dummy_ore, ts), handler(controller)
    {
        ok_ctx.mining_level = 15;
        ok_ctx.has_torch = true;
        ok_ctx.has_lantern = true;
    }
};

} // namespace

// CP-C-001 空闲态启动采矿：Idle → Mining
TEST(ContextGate, StartIdleEntersMining)
{
    GateFixture f;
    ASSERT_FALSE(f.handler.is_mining());
    EXPECT_TRUE(f.handler.start(0, f.ok_ctx));
    EXPECT_TRUE(f.handler.is_mining());
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Mining);
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 0);
}

// CP-C-002 采矿中重复启动（硬直拦截）：不覆盖会话
TEST(ContextGate, DoubleStartBlockedKeepsLayer)
{
    GateFixture f;
    ASSERT_TRUE(f.handler.start(0, f.ok_ctx));
    EXPECT_FALSE(f.handler.start(2, f.ok_ctx)); // 再次 start 返回 false
    ASSERT_TRUE(f.handler.layer_id().has_value());
    EXPECT_EQ(*f.handler.layer_id(), 0);        // 会话层未被覆盖
    EXPECT_TRUE(f.handler.is_mining());
}

// CP-C-007 等级不足拦截：核心层(需15) 而玩家仅 1
TEST(ContextGate, LevelInsufficientBlocked)
{
    GateFixture f;
    mining::MiningContext low;
    low.mining_level = 1;   // 无火把/灯笼
    EXPECT_FALSE(f.handler.start(4, low)); // core 需等级 15
    EXPECT_FALSE(f.handler.is_mining());
}

// CP-M-002/003 层号越界边界：≥5 均在控制器被拒绝
TEST(ContextGate, InvalidLayerIndexBlocked)
{
    GateFixture f;
    EXPECT_FALSE(f.handler.start(5, f.ok_ctx));
    EXPECT_FALSE(f.handler.start(99, f.ok_ctx));
    EXPECT_FALSE(f.handler.is_mining());
}

// CP-C-008 照明不足拦截：中层(火把) 无火把被拒，有火把放行
TEST(ContextGate, LightingTorchRequired)
{
    GateFixture f;
    mining::MiningContext no_light;
    no_light.mining_level = 15;
    no_light.has_torch = false;
    no_light.has_lantern = false;
    EXPECT_FALSE(f.handler.start(1, no_light)); // middle 需火把
    EXPECT_FALSE(f.handler.is_mining());

    EXPECT_TRUE(f.handler.start(1, f.ok_ctx));  // 有火把放行
}

// CP-C-008b 照明不足拦截：水晶层(灯笼) 无灯笼被拒
TEST(ContextGate, LightingLanternRequired)
{
    GateFixture f;
    mining::MiningContext torch_only;
    torch_only.mining_level = 15;
    torch_only.has_torch = true;   // 只有火把
    torch_only.has_lantern = false;
    EXPECT_FALSE(f.handler.start(3, torch_only)); // crystal 需灯笼

    EXPECT_TRUE(f.handler.start(3, f.ok_ctx));
}

// CP-C-009 停止后复位为 Idle，可再次采矿
TEST(ContextGate, StopResetsThenRestart)
{
    GateFixture f;
    ASSERT_TRUE(f.handler.start(2, f.ok_ctx));
    ASSERT_TRUE(f.handler.is_mining());

    const auto results = f.handler.stop(f.ok_ctx);
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Idle);
    EXPECT_FALSE(f.handler.layer_id().has_value());

    // stop 后 Idle 可再入，无残留脏状态
    EXPECT_TRUE(f.handler.start(0, f.ok_ctx));
    EXPECT_TRUE(f.handler.is_mining());
}

// CP-C-010 状态查询无副作用
TEST(ContextGate, StatusQueryReadOnly)
{
    GateFixture f;
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Idle);
    EXPECT_FALSE(f.handler.is_mining());
    EXPECT_TRUE(f.handler.start(0, f.ok_ctx));
    EXPECT_EQ(f.handler.status(), mining::MiningStatus::Mining);
    EXPECT_TRUE(f.handler.is_mining());
}

// CP-C-003 眩晕状态（stun）拦截 —— 命令层未实现，标注跳过
TEST(ContextGate, StunStateInterceptNotImplemented)
{
    GTEST_SKIP()
        << "眩晕(stun)状态对所有指令的拦截尚未在命令层实现（方案 CP-C-003）";
}

// CP-C-004 死亡状态（dead）拦截 —— 命令层未实现，标注跳过
TEST(ContextGate, DeadStateInterceptNotImplemented)
{
    GTEST_SKIP()
        << "死亡(dead)状态对任意指令的全网关拦截尚未实现（方案 CP-C-004）";
}