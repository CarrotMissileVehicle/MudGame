/**
 * @file TuiController.h
 * @brief TUI 交互控制器：命令解析 / 参数收集状态机 / 动态补全 / 每秒世界节拍。
 *
 * 从原 main.cpp 提取：process_line、completions、world_step、参数收集状态机
 * 与 worldMutex 均由本类持有。所有回调仅在主线程（FTXUI 事件循环）执行。
 */
#pragma once

#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

#include "connector.h"
#include "input_parser.h"

class GameContext;
class WorldEngine;
namespace mud::tui { class GameTui; }

/**
 * @brief TUI 交互控制器：驱动"回合制"REPL 与后台世界推进。
 *
 * 并发模型：worldMutex 串行化「后台时间推进」与「命令执行」，二者互斥访问
 * 共享世界状态，避免数据竞争。
 */
class TuiController
{
public:
    TuiController(GameContext& ctx, WorldEngine& world, Connector& connector);

    /// 接入 GameTui：设置 process_line / completions / tick 回调。
    void wire(mud::tui::GameTui& tui);

    /// 命令处理回调：返回 false 表示请求退出游戏循环。
    bool process_line(const std::string& raw_line);

    /// 动态补全回调：空闲按输入前缀过滤已知动词，收集中过滤当前参数候选。
    std::vector<std::string> completions(const std::string& input);

    /// 每秒后台节拍（经 post_background 在主线程执行）：推进世界 + 动作 + 刷新提示。
    void world_step();

private:
    void ask_next_param();      // 推进到下一个待填参数；全部填毕则 dispatch
    void start_collect(const std::string& verb); // 为某动词开始参数收集
    void cancel_collect();      // 取消进行中的参数收集
    void refresh_prompt();      // 刷新输入法提示（idle 时展示）

    GameContext& ctx_;
    WorldEngine& world_;
    Connector& connector_;
    InputParser parser_;
    HandlerContext handler_ctx_;   // 派发时透传的运行时依赖

    std::mutex worldMutex_;        // 串行化「后台推进」与「FTXUI 命令执行」

    // 参数收集状态
    mud::cmd::Command pendingCmd_;
    std::vector<mud::cmd::ParameterDef> pendingParams_;
    std::size_t pendingIndex_ = 0;
    bool collecting_ = false;
};