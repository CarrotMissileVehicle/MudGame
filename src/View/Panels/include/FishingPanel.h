/**
 * @file FishingPanel.h
 * @brief 钓鱼面板：渲染鱼池鱼类与概率、当前可钓性（收编 main 的 fish.status）。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class FishingPanel
    {
    public:
        explicit FishingPanel(Renderer& r) : r_(r) {}

        /// 渲染鱼池信息与可钓性。
        void render(const FishingView& fishing) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view