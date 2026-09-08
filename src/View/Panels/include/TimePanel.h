/**
 * @file TimePanel.h
 * @brief 时间面板：渲染当前游戏时间与时间倍率。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class TimePanel
    {
    public:
        explicit TimePanel(Renderer& r) : r_(r) {}

        /// 渲染当前时间（收编 print_now 的时间部分）。
        void render(const TimeView& t) const;

        /// 渲染时间倍率反馈：`时间倍率设为 X。`
        void render_scale(double factor) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view