/**
 * @file connector.h
 * @brief 命令派发器（Connector）及其伴随类型定义。
 *
 * 数据流总览：
 *   line → InputParser::parse → Command(点分 verb)
 *        → Connector::dispatch → (按 verb 查表) → 领域 handler
 *        → HandlerResult → main 打印反馈
 *
 * 依赖：time_service、mining_state、input_parser。
 */
#pragma once

#include "time_service.h"
#include "mining_state.h"
#include "input_parser.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

/** @brief 命令处理结果枚举，用于向调用方反馈执行状态。 */
enum class HandlerResult { Ok, UnknownCommand, BadArgument, Failed };

/**
 * @brief 派发给领域 handler 的运行时依赖集合。
 *
 * 以引用透传共享状态，时间服务为非常量引用以支持设置时间倍率。
 */
struct HandlerContext
{
    // 透传给领域 handler 的运行时依赖
    mud::TimeService& time; // 非 const：time scale 需调用 set_time_scale
    ::MiningState& session;
};

/** @brief 命令处理器回调类型：接收命令与上下文，返回处理结果。 */
using Handler = std::function<
    HandlerResult(const mud::cmd::Command&, const HandlerContext&)>;

/**
 * @brief 命令派发器：维护 verb → handler 的映射表并负责命令路由。
 *
 * 通过 bind() 注册处理器，dispatch() 依据命令的点分 verb 查找并执行。
 */
class Connector
{
private:
    std::unordered_map<std::string, Handler> handlers_; // verb → handler 映射

public:
    /** @brief 绑定指定命令 verb 到处理器回调。 */
    void bind(std::string verb, Handler handler);

    /** @brief 按命令 verb 派发执行，未命中时返回 UnknownCommand。 */
    HandlerResult dispatch(
        const mud::cmd::Command& command,
        const HandlerContext& ctx) const;

    /** @brief 查询指定 verb 是否已有绑定的处理器。 */
    bool has(std::string_view verb) const;
};
