/**
 * @file connector.cpp
 * @brief 命令派发器实现。
 *
 * 实现 verb→handler 的注册、大小写不敏感的查找与派发逻辑。
 * 交互模式下自动查 schema → 调用 ParameterCollector → 执行 handler。
 */
#include "connector.h"
#include "input_parser.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace
{
    // 统一 verb 大小写：所有注册与查找路径共用，保证大小写不敏感路由一致
    std::string normalize_verb(std::string_view verb)
    {
        std::string key(verb);
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return key;
    }
}

/**
 * @brief 注册命令处理器。
 * @param verb    命令动词（点分形式），保存前统一转为小写。
 * @param handler 对应处理器回调。
 *
 * 相同 verb 重复绑定会覆盖旧的处理器。
 */
void Connector::bind(std::string verb, Handler handler)
{
    // 统一小写，保证后续大小写不敏感匹配。
    handlers_[normalize_verb(verb)] = std::move(handler);
}

/**
 * @brief 注册命令参数 Schema（交互模式下使用）。
 * @param verb   命令动词，保存前统一转为小写。
 * @param schema 参数定义。
 */
void Connector::register_schema(std::string verb, mud::cmd::CommandSchema schema)
{
    schemas_[normalize_verb(verb)] = std::move(schema);
}

/**
 * @brief 设置交互式参数收集器。
 * @param collector 参数收集器指针（可为 nullptr 表示禁用交互收集）。
 */
void Connector::set_collector(ParameterCollector* collector)
{
    collector_ = collector;
}

/**
 * @brief 派发命令到对应的处理器执行（交互模式）。
 *
 * 流程：统一小写 verb → 查 schema → 若有 schema 且 collector 可用，
 * 调用 collector 收集缺失参数 → 查 handler → 执行。
 */
HandlerResult Connector::dispatch(const mud::cmd::Command& command,
                                  const HandlerContext& ctx) const
{
    // 同样统一小写后再查表，实现大小写不敏感路由。
    const std::string verb = normalize_verb(command.verb);

    // 查找 handler
    auto handler_it = handlers_.find(verb);
    if (handler_it == handlers_.end()) return HandlerResult::UnknownCommand;

    // 交互模式：若有 schema 且 collector 可用，收集缺失参数
    mud::cmd::Command cmd = command; // 允许修改副本
    auto schema_it = schemas_.find(verb);
    if (schema_it != schemas_.end() && collector_)
    {
        if (!collector_->collect(schema_it->second, cmd))
            return HandlerResult::Failed; // 用户取消
    }

    return handler_it->second(cmd, ctx);
}

/**
 * @brief 判断指定 verb 是否已注册处理器（大小写不敏感，与 dispatch 一致）。
 */
bool Connector::has(std::string_view verb) const
{
    return handlers_.find(normalize_verb(verb)) != handlers_.end();
}

/**
 * @brief 判断指定 verb 是否已注册 schema（大小写不敏感，与 dispatch 一致）。
 */
bool Connector::has_schema(std::string_view verb) const
{
    return schemas_.find(normalize_verb(verb)) != schemas_.end();
}

/**
 * @brief 获取已注册的 schema（用于生成帮助文本）。
 *
 * @pre verb 必须已注册（先经 has_schema() 校验）；未注册时抛 std::out_of_range。
 * @note 返回的引用指向 schemas_ 内部元素，在后续 register_schema() 触发重哈希后可能失效，
 *       不应跨注册边界长期持有。
 */
const mud::cmd::CommandSchema& Connector::get_schema(std::string_view verb) const
{
    return schemas_.at(normalize_verb(verb));
}
