/**
 * @file MessagePanel.cpp
 * @brief 消息面板实现。
 */
#include "MessagePanel.h"

namespace mud::view
{
    void MessagePanel::render(const MessageLine& msgs) const
    {
        for (const auto& m : msgs)
            r_.print(m);
    }
} // namespace mud::view