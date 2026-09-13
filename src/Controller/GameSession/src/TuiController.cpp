/**
 * @file TuiController.cpp
 * @brief TuiController 实现：REPL 状态机与每秒世界节拍（源自原 main.cpp）。
 */
#include "TuiController.h"

#include <mutex>

#include "GameContext.h"
#include "WorldEngine.h"
#include "CommandCatalog.h"
#include "GameTui.h"

TuiController::TuiController(GameContext& ctx, WorldEngine& world, Connector& connector)
    : ctx_(ctx), world_(world), connector_(connector),
      handler_ctx_{ctx_.time, ctx_.mining}
{
}

void TuiController::wire(mud::tui::GameTui& tui)
{
    tui.set_process_line([this](const std::string& line) { return process_line(line); });
    tui.set_completions([this](const std::string& input) { return completions(input); });
    tui.set_tick([this]() { refresh_prompt(); });
}

void TuiController::refresh_prompt()
{
    ctx_.refresh_prompt();
}

// 推进到下一个待填参数；全部填毕则 dispatch。（调用方需已持有 worldMutex）
void TuiController::ask_next_param()
{
    if (pendingIndex_ >= pendingParams_.size()) {
        collecting_ = false;
        ctx_.tui.question.clear();
        ctx_.tui.completions.clear();
        const std::string verb = pendingCmd_.verb;
        HandlerResult r = connector_.dispatch(pendingCmd_, handler_ctx_);
        pendingCmd_ = mud::cmd::Command{};
        pendingParams_.clear();
        pendingIndex_ = 0;
        if (r == HandlerResult::UnknownCommand)
            ctx_.msg("未知命令：" + verb + "（输入 help 查看）");
        return;
    }
    const auto& p = pendingParams_[pendingIndex_];
    ctx_.tui.question = "请输入 " + p.name + "：" + p.prompt;
    ctx_.tui.completions = CommandCatalog::param_choices(p, ctx_);
}

// 开始为某个动词收集参数（调用方需已持有 worldMutex）。
void TuiController::start_collect(const std::string& verb)
{
    collecting_ = true;
    pendingCmd_ = mud::cmd::Command{};
    pendingCmd_.verb = verb;
    pendingCmd_.raw = verb;
    pendingParams_ = connector_.get_schema(verb).parameters;
    pendingIndex_ = 0;
    ask_next_param();
}

// 用户停止需要 min 参数的命令时取消收集
void TuiController::cancel_collect()
{
    collecting_ = false;
    pendingCmd_ = mud::cmd::Command{};
    pendingParams_.clear();
    pendingIndex_ = 0;
    ctx_.tui.question.clear();
    ctx_.tui.completions.clear();
}

// TUI 命令处理回调：返回 true 表示命令已处理（继续运行）；false 表示请求退出循环。
bool TuiController::process_line(const std::string& raw_line)
{
    std::string line = raw_line;
    while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
        line.erase(line.begin());
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
        line.pop_back();
    if (line.empty())
        return true;

    // q：参数收集中则取消输入；否则结束当前进行的动作
    if (line == "q" || line == "Q") {
        if (collecting_) {
            cancel_collect();
            ctx_.msg("已取消输入。");
            return true;
        }
        std::lock_guard<std::mutex> lock(worldMutex_);
        if (ctx_.tui.action_type == mud::tui::ActionType::Fish) {
            world_.end_fishing(true);
            return true;
        }
        if (ctx_.mining.is_mining()) {
            mud::cmd::Command cmd;
            cmd.verb = "mine.stop";
            cmd.raw = line;
            connector_.dispatch(cmd, handler_ctx_);
            return true;
        }
        ctx_.msg("没有正在进行的动作。");
        return true;
    }

    {
        std::lock_guard<std::mutex> lock(worldMutex_);

        // 参数收集进行中：将本行作为当前参数的值。
        if (collecting_) {
            if (line == "quit") {
                cancel_collect();
                ctx_.msg("已取消输入。");
                return true;
            }
            const auto& p = pendingParams_[pendingIndex_];
            if (!line.empty()) {
                pendingCmd_.options[p.name] = line;
                ++pendingIndex_;
            } else if (!p.default_value.empty()) {
                pendingCmd_.options[p.name] = p.default_value;
                ++pendingIndex_;
            } else if (!p.required) {
                ++pendingIndex_;
            } else {
                ask_next_param(); // 必填且无默认：重新提示
                return true;
            }
            ask_next_param();
            return true;
        }

        const std::string verb = parser_.parse_verb_only(line);

        if (verb == "quit") {
            ctx_.tui.push_log("再见！");
            return false; // 通知 GameTui 退出
        }
        if (verb == "help") {
            ctx_.view.render_message(mud::view::MessageLine{CommandCatalog::help_text()});
            return true;
        }
        if (verb.empty()) {
            ctx_.msg("无法识别的输入：" + line);
            return true;
        }

        // 交互收集：注册了 schema 的动词先逐参数提示，不再直接执行。
        if (connector_.has_schema(verb)) {
            start_collect(verb);
            return true;
        }

        mud::cmd::Command cmd;
        cmd.verb = verb;
        cmd.raw = line;
        HandlerResult result = connector_.dispatch(cmd, handler_ctx_);
        if (result == HandlerResult::UnknownCommand)
            ctx_.msg("未知命令：" + verb + "（输入 help 查看）");
    }
    return true;
}

// TUI 动态补全回调：空闲按输入前缀过滤已知动词；参数收集中过滤当前参数的候选。
std::vector<std::string> TuiController::completions(const std::string& input)
{
    if (input.empty())
        return {};
    if (collecting_)
        return CommandCatalog::param_choices(pendingParams_[pendingIndex_], ctx_);
    std::vector<std::string> out;
    for (const auto& v : CommandCatalog::known_verbs())
        if (v.rfind(input, 0) == 0)
            out.push_back(v);
    return out;
}

// 每秒后台节拍：推进世界 + 推进非阻塞动作 + 刷新 idle 提示。
void TuiController::world_step()
{
    std::lock_guard<std::mutex> lock(worldMutex_);
    world_.tick_world();
    world_.handle_action_tick();
    refresh_prompt();
}