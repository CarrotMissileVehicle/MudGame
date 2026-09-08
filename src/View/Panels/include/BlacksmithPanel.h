/**
 * @file BlacksmithPanel.h
 * @brief 铁匠铺面板：展示各工具修复需求的矿石与金币费用（含背包持有矿石数）。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class BlacksmithPanel
    {
    public:
        explicit BlacksmithPanel(Renderer& r) : r_(r) {}

        /// 渲染铁匠铺修复信息。
        void render(const BlacksmithView& view) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view