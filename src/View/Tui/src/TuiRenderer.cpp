/**
 * @file TuiRenderer.cpp
 * @brief 将 View 层输出重定向到 TUI 日志区。
 */
#include "TuiRenderer.h"

#include "TuiState.h"

namespace mud::tui
{
    void TuiRenderer::print(const std::string& line)
    {
        // print_raw 不换行；print 追加逻辑行（去掉末尾换行符，统一入日志）
        std::string out = line;
        if (!out.empty() && out.back() == '\n')
            out.pop_back();
        state_.push_log(out);
    }

    void TuiRenderer::print_raw(const std::string& s)
    {
        // 提示符等原始输出不需要进入日志（FTXUI 输入框自带提示符）。
        (void)s;
    }
} // namespace mud::tui
