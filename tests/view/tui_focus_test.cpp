/**
 * @file tui_focus_test.cpp
 * @brief GameTui 输入焦点与命令链路测试（不进入阻塞式事件循环）。
 *
 * 通过离线调用 GameTui::build() 构造组件树，再向顶层组件注入
 * ftxui::Event，断言：
 *   - 键盘字符能进入输入框（初始焦点位于输入框，而非日志区）；
 *   - Enter 提交后能触发 process_line 回调；
 *   - 连续多条命令可正常依次执行；
 *   - 空输入不触发 dispatch。
 *
 * 这是"黑盒 + 灰盒"测试：只注入外部输入并观察 TuiState 与回调副作用，
 * 不依赖 FTXUI 内部渲染细节。
 */
#include <gtest/gtest.h>

#include "GameTui.h"
#include "TuiState.h"

#include <ftxui/component/event.hpp>

#include <memory>

using namespace mud::tui;

namespace
{
    // 构造一个已 build（不 run）的 GameTui 夹具。
    struct TuiFixture
    {
        TuiState state;
        std::vector<std::string> dispatched;
        bool process_called = false;

        std::unique_ptr<GameTui> make()
        {
            auto tui = std::make_unique<GameTui>(state);
            tui->set_process_line([this](const std::string& line) -> bool {
                process_called = true;
                dispatched.push_back(line);
                return true;
            });
            tui->set_completions([](const std::string&) -> std::vector<std::string> {
                return {};
            });
            tui->build();
            return tui;
        }
    };
} // namespace

// 初始焦点应在输入框：注入可打印字符应进入 state_.input。
TEST(TuiFocusTest, CharacterReachesInputOnBuild)
{
    TuiFixture fx;
    auto tui = fx.make();
    ASSERT_NE(tui->component_root(), nullptr);

    const auto root = tui->component_root();
    EXPECT_TRUE(root->OnEvent(ftxui::Event::Character("h")));
    EXPECT_EQ(fx.state.input, "h");

    EXPECT_TRUE(root->OnEvent(ftxui::Event::Character('o')));
    EXPECT_EQ(fx.state.input, "ho");
}

// 空输入时无需处理命令。
TEST(TuiFocusTest, EnterOnEmptyInputNoOp)
{
    TuiFixture fx;
    auto tui = fx.make();
    const auto root = tui->component_root();

    EXPECT_TRUE(root->OnEvent(ftxui::Event::Return));
    EXPECT_FALSE(fx.process_called);
}

// 输入文本后按 Enter，应触发 process_line 且输入框被清空。
TEST(TuiFocusTest, EnterSubmitsLineAndClearsInput)
{
    TuiFixture fx;
    auto tui = fx.make();
    const auto root = tui->component_root();

    for (char c : std::string("help"))
        root->OnEvent(ftxui::Event::Character(c));

    EXPECT_EQ(fx.state.input, "help");
    EXPECT_TRUE(root->OnEvent(ftxui::Event::Return));
    EXPECT_TRUE(fx.process_called);
    EXPECT_EQ(fx.dispatched, std::vector<std::string>({"help"}));
    EXPECT_TRUE(fx.state.input.empty());
}

// 连续多条命令应依次正常执行。
TEST(TuiFocusTest, MultipleCommandsRunSequentially)
{
    TuiFixture fx;
    auto tui = fx.make();
    const auto root = tui->component_root();

    for (const char* cmd : {"time.now", "player.status"})
    {
        for (const char* p = cmd; *p; ++p)
            root->OnEvent(ftxui::Event::Character(*p));
        root->OnEvent(ftxui::Event::Return);
    }

    ASSERT_EQ(fx.dispatched.size(), 2u);
    EXPECT_EQ(fx.dispatched[0], "time.now");
    EXPECT_EQ(fx.dispatched[1], "player.status");
}