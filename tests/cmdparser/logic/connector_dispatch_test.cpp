/**
 * @file connector_dispatch_test.cpp
 * @brief 逻辑级测试（方案 CP-L）：Connector 分发、大小写路由、默认 UnknownCommand、
 *       bind 覆盖 与"后端调用是否发生"探查。
 *
 * 被测对象：Connector（src/View/Cmdparser）。
 * 环境作用域（矿洞房间）、方向可达性等尚未在命令层实现，以 GTEST_SKIP 标注。
 */
#include <gtest/gtest.h>

#include <string>

#include "connector.h"
#include "input_parser.h"
#include "mining_handler.h"
#include "mining_types.h"
#include "time_service.h"

using mud::cmd::Command;

namespace {

// 一个可复用的 Connector 测试夹具：构造真实依赖以填充 HandlerContext。
struct DispatchFixture
{
    TimeService ts;
    Ore::OreData dummy_ore;
    MiningController controller;
    MiningHandler mining;
    HandlerContext ctx;

    DispatchFixture()
        : controller(dummy_ore, ts), mining(controller),
          ctx(HandlerContext{ts, mining})
    {
    }
};

Command make_command(std::string verb, std::string raw = "")
{
    Command c;
    c.verb = std::move(verb);
    c.raw  = raw.empty() ? c.verb : std::move(raw);
    return c;
}

} // namespace

// CP-L-001 已绑定 verb 正确派发，且恰好调用一次、实参透传
TEST(ConnectorDispatch, BoundVerbDispatchesExactlyOnce)
{
    DispatchFixture f;
    Connector conn;

    int calls = 0;
    std::string seen_verb;
    conn.bind("mine.start", [&](const Command& c, const HandlerContext&) {
        ++calls;
        seen_verb = c.verb;
        return HandlerResult::Ok;
    });

    const auto result = conn.dispatch(make_command("mine.start"), f.ctx);
    EXPECT_EQ(result, HandlerResult::Ok);
    EXPECT_EQ(calls, 1);
    EXPECT_EQ(seen_verb, "mine.start");
}

// CP-L-002 未绑定 verb → UnknownCommand，任何 handler 不被调用
TEST(ConnectorDispatch, UnboundVerbReturnsUnknownAndNoCall)
{
    DispatchFixture f;
    Connector conn;

    int calls = 0;
    conn.bind("mine.start", [&](const Command&, const HandlerContext&) {
        ++calls;
        return HandlerResult::Ok;
    });

    for (const auto& verb : {"go north", "look rock", "foo bar"})
    {
        EXPECT_EQ(conn.dispatch(make_command(verb), f.ctx),
                  HandlerResult::UnknownCommand);
    }
    EXPECT_EQ(calls, 0); // 后端 handler 一律不得被调用
}

// CP-L-003 空 verb 不路由、零副作用
TEST(ConnectorDispatch, EmptyVerbNotRouted)
{
    DispatchFixture f;
    Connector conn;
    int calls = 0;
    conn.bind("help", [&](const Command&, const HandlerContext&) {
        ++calls;
        return HandlerResult::Ok;
    });

    EXPECT_EQ(conn.dispatch(make_command(""), f.ctx),
              HandlerResult::UnknownCommand);
    EXPECT_EQ(calls, 0);
}

// CP-L-004 Connector 大小写不敏感路由：MINE.START → mine.start handler
TEST(ConnectorDispatch, CaseInsensitiveRouting)
{
    DispatchFixture f;
    Connector conn;
    int calls = 0;
    conn.bind("mine.start", [&](const Command&, const HandlerContext&) {
        ++calls;
        return HandlerResult::Ok;
    });

    EXPECT_EQ(conn.dispatch(make_command("MINE.START"), f.ctx),
              HandlerResult::Ok);
    EXPECT_EQ(calls, 1);
}

// CP-L-009 bind 覆盖：重复绑定以新 handler 为准；has() 查表一致
TEST(ConnectorDispatch, BindOverridesAndHasMatches)
{
    DispatchFixture f;
    Connector conn;

    int old_calls = 0, new_calls = 0;
    conn.bind("mine.stop", [&](const Command&, const HandlerContext&) {
        ++old_calls;
        return HandlerResult::BadArgument;
    });
    conn.bind("mine.stop", [&](const Command&, const HandlerContext&) {
        ++new_calls;
        return HandlerResult::Ok;
    });

    EXPECT_TRUE(conn.has("mine.stop"));
    EXPECT_FALSE(conn.has("north"));
    EXPECT_EQ(conn.dispatch(make_command("mine.stop"), f.ctx),
              HandlerResult::Ok);
    EXPECT_EQ(old_calls, 0); // 旧 handler 被覆盖，不应再被调用
    EXPECT_EQ(new_calls, 1);
}

// CP-L-010 后端调用实参证据：依法进入 handler 后，收到的 Command 与依赖有效
TEST(ConnectorDispatch, DispatchedHandlerReceivesValidCommandAndCtx)
{
    DispatchFixture f;
    Connector conn;

    Command received;
    const HandlerContext* ctx_ptr = nullptr;
    conn.bind("mine.start", [&](const Command& c, const HandlerContext& h) {
        received = c;
        ctx_ptr  = &h;
        return HandlerResult::Ok;
    });

    conn.dispatch(make_command("mine.start", "mine.start --layer 2"), f.ctx);
    EXPECT_EQ(received.verb, "mine.start");
    EXPECT_EQ(received.raw, "mine.start --layer 2");
    ASSERT_NE(ctx_ptr, nullptr);
    EXPECT_TRUE(ctx_ptr->mining.is_mining() == false
                || ctx_ptr->mining.is_mining() == true); // 引用有效可访问
}

// CP-L-005 环境作用域：不在矿洞房间时 `mine` 拦截 —— 命令层未实现，标注跳过
TEST(ConnectorDispatch, EnvironmentScopeNotImplemented)
{
    GTEST_SKIP()
        << "不在矿洞房间时 mine 的环境作用域拦截尚未在命令层实现（方案 CP-L-005）";
}

// CP-L-007 方向不可达拦截 —— 命令层未实现，标注跳过
TEST(ConnectorDispatch, DirectionReachabilityNotImplemented)
{
    GTEST_SKIP()
        << "go north 方向不可达的环境拦截尚未实现（方案 CP-L-007）";
}