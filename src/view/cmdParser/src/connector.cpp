/**
 * @file connector.cpp
 * @brief 命令派发器实现。
 *
 * 实现 verb→handler 的注册、大小写不敏感的查找与派发逻辑。
 */
#include "connector.h"
#include "input_parser.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

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
    std::transform(verb.begin(), verb.end(), verb.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    handlers_[std::move(verb)] = std::move(handler);
}

/**
 * @brief 派发命令到对应的处理器执行。
 * @param command 待执行命令。
 * @param ctx     透传给处理器的运行时上下文。
 * @return 处理器返回的结果；未命中时返回 UnknownCommand。
 */
HandlerResult Connector::dispatch(const mud::cmd::Command& command,
                                  const HandlerContext& ctx) const
{
    // 同样统一小写后再查表，实现大小写不敏感路由。
    std::string verb = command.verb;
    std::transform(verb.begin(), verb.end(), verb.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    auto it = handlers_.find(verb);
    if (it == handlers_.end()) return HandlerResult::UnknownCommand;
    return it->second(command, ctx);
}

/**
 * @brief 判断指定 verb 是否已注册处理器。
 * @param verb 命令动词。
 * @return 已注册返回 true，否则返回 false。
 */
bool Connector::has(std::string_view verb) const
{
    return handlers_.find(std::string(verb)) != handlers_.end();
}
