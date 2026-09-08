/**
 * @file MarketPanel.h
 * @brief 集市面板：渲染金币、集市日与各商店货架行情（收编 main 的 market.status）。
 */
#pragma once

#include "Renderer.h"
#include "dto.h"

namespace mud::view
{
    class MarketPanel
    {
    public:
        explicit MarketPanel(Renderer& r) : r_(r) {}

        /// 渲染集市行情面板。
        void render(const MarketView& market) const;

    private:
        Renderer& r_;
    };
} // namespace mud::view