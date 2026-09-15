/**
 * @file HelpScreen.cpp
 * @brief 帮助屏实现。
 */
#include "HelpScreen.h"

namespace mud::view
{
    void HelpScreen::render(const std::string& help_text) const
    {
        // 帮助文本逐行经 print() 输出：TuiRenderer 也能把内容写入日志区，
        // 而不是经 print_raw 丢弃（print_raw 仅用于提示符等原始输出）。
        std::size_t begin = 0;
        while (begin < help_text.size())
        {
            const std::size_t end = help_text.find('\n', begin);
            r_.print(help_text.substr(begin, end - begin));
            if (end == std::string::npos)
                break;
            begin = end + 1;
        }
    }
} // namespace mud::view