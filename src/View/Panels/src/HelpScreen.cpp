/**
 * @file HelpScreen.cpp
 * @brief 帮助屏实现。
 */
#include "HelpScreen.h"

namespace mud::view
{
    void HelpScreen::render(const std::string& help_text) const
    {
        r_.print_raw(help_text);
        if (help_text.empty() || help_text.back() != '\n')
            r_.print("");
    }
} // namespace mud::view