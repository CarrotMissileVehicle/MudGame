/**
 * @file GameTui.cpp
 * @brief 基于 FTXUI v7 的终端图形界面实现。
 *
 * 布局：
 *   ┌─────────────────────────────────┐
 *   │ 日志区（scrollable，占满剩余）  │
 *   ├─────────────────────────────────┤
 *   │ [候选项列表（有输入时显示）]     │
 *   │  问题提示行                     │
 *   │ > 输入框                       │
 *   └─────────────────────────────────┘
 *
 * 交互：Enter 执行命令；Ctrl+C 退出。
 * 后台线程通过 post_background 在主线程执行 tick_world 等回调。
 */
#include "GameTui.h"

#include "LogView.h"

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include <algorithm>
#include <chrono>
#include <string>

using namespace ftxui;

namespace mud::tui
{
    struct GameTui::Impl
    {
        TuiState& state;

        // FTXUI 组件（持有生命周期）
        Component input_component;
        std::shared_ptr<LogView> log_view;
        Component completions_renderer;
        Component question_renderer;
        Component hint_renderer;
        Component root;          // 内层：日志 + 底部输入区
        Component top;           // 最终顶层：CatchEvent 包装 + 布局 Renderer

        explicit Impl(TuiState& s) : state(s) {}
    };

    // ========================================================================
    // 构造 / 析构 / 设置回调
    // ========================================================================
    GameTui::GameTui(TuiState& state)
        : state_(state), impl_(std::make_unique<Impl>(state)) {}

    GameTui::~GameTui() = default;

    void GameTui::set_process_line(std::function<bool(const std::string&)> fn)
    {
        process_line_ = std::move(fn);
    }

    void GameTui::set_completions(std::function<std::vector<std::string>(const std::string&)> fn)
    {
        completions_ = std::move(fn);
    }

    void GameTui::set_tick(std::function<void()> fn)
    {
        tick_ = std::move(fn);
    }

    void GameTui::exit()
    {
        if (app_) app_->Exit();
    }

    void GameTui::post_background(std::function<void()> fn)
    {
        if (app_) app_->PostEventOrExecute(std::move(fn));
    }

    // ========================================================================
    // build()：构建 FTXUI 组件树（不进入事件循环）
    // ========================================================================
    void GameTui::build()
    {
        // ---- 动态完成列表（输入框上方，输入为空时隐藏）----
        impl_->completions_renderer = Renderer([&]() -> Element {
            if (state_.completions.empty())
                return text("");
            Elements items;
            for (const auto& c : state_.completions)
                items.push_back(text("  " + c));
            return window(text("候选项"), vbox(std::move(items)));
        });

        // ---- 问题提示行（输入框上方）----
        impl_->question_renderer = Renderer([&]() -> Element {
            if (state_.question.empty())
                return text("");
            return hbox({text(" " + state_.question) | bold});
        });

        // ---- 输入法提示（idle 且无补全时显示）----
        impl_->hint_renderer = Renderer([&]() -> Element {
            if (state_.input_hint.empty() || !state_.input.empty())
                return text("");
            if (!state_.completions.empty())
                return text("");
            return hbox({text(state_.input_hint) | dim});
        });

        // ---- 输入框 ----
        // 构建 InputOption，显式设置回调（避免 InputOption::Default 的
        // multiline=true 与 std::string 内部字段产生歧义）。
        // 输入缓冲直接绑定 TuiState::input，供外部回调与布局逻辑读取。
        InputOption input_opt;
        input_opt.content = &state_.input;
        input_opt.placeholder = "输入命令（help 查看帮助）";
        input_opt.multiline = false;

        // on_enter：用户按下 Enter 时处理一行命令输入
        input_opt.on_enter = [&]() {
            const std::string line = state_.input;
            state_.input.clear();
            state_.completions.clear();

            if (line.empty())
            {
                if (tick_) tick_();
                return;
            }

            // 直接退出命令（不走 dispatch）
            if (line == "quit" || line == "exit")
            {
                state_.push_log("再见！");
                exit();
                return;
            }

            // 调用外部注册的命令处理回调
            if (process_line_)
                process_line_(line);

            // 新输入已提交：日志强制回到最下方
            impl_->log_view->force_follow();

            // 处理完后刷新补全列表
            if (completions_)
                state_.completions = completions_(state_.input);

            if (tick_) tick_();
        };

        // on_change：输入内容变化时实时更新补全列表
        input_opt.on_change = [&]() {
            if (completions_)
                state_.completions = completions_(state_.input);
        };

        impl_->input_component = Input(input_opt);

        // ---- 日志区（可滚动 + 跟随底部）----
        impl_->log_view = std::make_shared<LogView>(state_);

        // ---- 组件树：日志区 + 底部区域 ----
        impl_->root = Container::Vertical({
            impl_->log_view,              // 日志：占满上方，处理鼠标滚轮
            Container::Vertical({         // 底部：自上而下
                impl_->completions_renderer,
                impl_->question_renderer,
                impl_->input_component,   // 输入框：焦点默认在这里
            }),
        });

        // ---- 事件捕获：Ctrl+C 退出；Tab 采用首个候选项 ----
        auto event_root = CatchEvent(impl_->root, [&](Event event) -> bool {
            if (event == Event::CtrlC)
            {
                exit();
                return true;
            }
            // Tab：将首个候选项填入输入框（二次按可继续选下一个？此处仅首个）
            if (event == Event::Tab)
            {
                if (completions_)
                    state_.completions = completions_(state_.input);
                if (!state_.completions.empty())
                {
                    state_.input = state_.completions.front();
                    if (completions_)
                        state_.completions = completions_(state_.input);
                    return true;
                }
            }
            return false;
        });

        // ---- 终端渲染器：组合最终布局 ----
        auto layout_renderer = Renderer(event_root, [&]() -> Element {
            // 日志：占据全部剩余空间（yflex）
            auto log_el = impl_->log_view->Render() | yflex;

            // 底部区域（自上而下：候选项 / 问题行 / 输入框 / idle 提示）
            // 规则：参数收集时显示问题行（+候选项）；idle 且无输入时显示提示；
            //       idle 且有输入时按输入显示候选项。
            Elements bottom;
            if (!state_.question.empty())
            {
                if (!state_.completions.empty())
                    bottom.push_back(impl_->completions_renderer->Render());
                bottom.push_back(impl_->question_renderer->Render());
            }
            else if (state_.input.empty())
            {
                bottom.push_back(impl_->hint_renderer->Render());
            }
            else if (!state_.completions.empty())
            {
                bottom.push_back(impl_->completions_renderer->Render());
            }
            bottom.push_back(impl_->input_component->Render());

            return vbox({
                log_el,
                separator(),
                vbox(std::move(bottom)),
            });
        });

        // ---- 初始焦点定向到输入框（根因修复）----
        // FTXUI 键盘事件只派发到"当前聚焦组件"所在的焦点链；组件树初始
        // selected_=0 使焦点落在日志区（日志区无子组件、Focusable()==false），
        // 字符事件在 container.cpp 的 `if (!Focused()) return false` 处被整体
        // 丢弃，导致"界面正常、有音乐、却无法输入命令"。TakeFocus() 沿 parent
        // 链把每个容器的焦点指向 input_component，恢复键盘输入。
        impl_->input_component->TakeFocus();

        // 保存最终顶层组件（事件/渲染根），供 component_root() 与 run() 复用。
        impl_->top = layout_renderer;
    }

    // ========================================================================
    // component_root() / run()
    // ========================================================================
    ftxui::Component GameTui::component_root()
    {
        return impl_->top;
    }

    void GameTui::run()
    {
        build();
        app_ = std::make_unique<ftxui::App>(ftxui::App::Fullscreen());
        app_->Loop(impl_->top);
    }
} // namespace mud::tui
