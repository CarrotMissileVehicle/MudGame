/**
 * @file MarketPanel.cpp
 * @brief 集市面板实现。
 */
#include "MarketPanel.h"

namespace mud::view
{
    void MarketPanel::render(const MarketView& market) const
    {
        r_.print("金币：" + std::to_string(market.gold));
        std::string dayLine = "集市日：周" + std::to_string(market.day_of_week);
        if (market.prosperous) dayLine += "（繁华集）";
        if (market.festival) dayLine += "（节日集）";
        r_.print(dayLine);
        for (const auto& shop : market.shops)
        {
            r_.print("[" + shop.id + "] " + shop.name);
            for (std::size_t j = 0; j < shop.items.size(); ++j)
            {
                r_.print("  [" + std::to_string(j) + "] " + shop.items[j].name + "：买 " +
                         std::to_string(shop.items[j].buy) + " / 卖 " +
                         std::to_string(shop.items[j].sell));
            }
        }
    }
} // namespace mud::view