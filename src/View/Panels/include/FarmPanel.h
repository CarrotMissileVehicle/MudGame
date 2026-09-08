/**
 * @file FarmPanel.h
 * @brief 农田面板：逐地块渲染种植状态（收编 main 的 farm.status）。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class FarmPanel
    {
    public:
        explicit FarmPanel(Renderer& r) : r_(r) {}

        /// 渲染全部地块状态。
        void render(const FarmView& farm) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view