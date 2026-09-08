/**
 * @file connector.h
 * @brief 命令派发器（Connector）及其伴随类型定义。
 *
 * 数据流总览（交互模式）：
 *   line → InputParser::parse_verb_only → verb
 *        → Connector::dispatch → 查 schema → ParameterCollector 逐参数提示
 *        → Command(options 已填充) → handler → HandlerResult → main 打印反馈
 *
 * 依赖：time_service、mining_handler、input_parser。
 */
#pragma once

#include "time_service.h"
#include "mining_handler.h"
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
 * MVC 联动：View 层经由 Controller 层（MiningHandler）驱动采矿业务，
 * 时间服务为非常量引用以支持设置时间倍率。
 */
struct HandlerContext
{
    // 透传给领域 handler 的运行时依赖
    mud::TimeService& time; // 非 const：time scale 需调用 set_time_scale
    MiningHandler& mining;  // 采矿命令入口（内含会话状态，View 不直接触碰状态）
};

/** @brief 命令处理器回调类型：接收命令与上下文，返回处理结果。 */
using Handler = std::function<
    HandlerResult(const mud::cmd::Command&, const HandlerContext&)>;

/**
 * @brief 命令派发器：维护 verb → handler 的映射表并负责命令路由。
 *
 * 交互模式下：dispatch 自动查 schema → 调用 ParameterCollector 收集参数 → 再执行 handler。
 */
class Connector
{
private:
    std::unordered_map<std::string, Handler> handlers_; // verb → handler 映射
    std::unordered_map<std::string, mud::cmd::CommandSchema> schemas_; // verb → 参数 schema
    ParameterCollector* collector_ = nullptr; // 交互式参数收集器（外部注入）

public:
    /** @brief 注册命令处理器。 */
    void bind(std::string verb, Handler handler);

    /** @brief 注册命令参数 Schema（交互模式下使用）。 */
    void register_schema(std::string verb, mud::cmd::CommandSchema schema);

    /** @brief 设置交互式参数收集器。 */
    void set_collector(ParameterCollector* collector);

    /**
     * @brief 按命令 verb 派发执行（交互模式）。
     *
     * 若已注册 schema，自动调用 ParameterCollector 收集缺失参数后再执行 handler。
     * 未命中时返回 UnknownCommand。
     */
    HandlerResult dispatch(
        const mud::cmd::Command& command,
        const HandlerContext& ctx) const;

    /** @brief 查询指定 verb 是否已有绑定的处理器。 */
    bool has(std::string_view verb) const;

    /** @brief 查询指定 verb 是否已注册 schema。 */
    bool has_schema(std::string_view verb) const;

    /** @brief 获取已注册的 schema（用于生成帮助文本）。 */
    const mud::cmd::CommandSchema& get_schema(std::string_view verb) const;
};
