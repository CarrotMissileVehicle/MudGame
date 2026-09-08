/**
 * @file MessagePanel.h
 * @brief 消息面板：Controller 把动作结果文本交给 View，View 逐行展示。
 */
#pragma once

#include <vector>

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class MessagePanel
    {
    public:
        explicit MessagePanel(Renderer& r) : r_(r) {}

        /// 逐行渲染反馈消息（空列表则无输出）。
        void render(const MessageLine& msgs) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view