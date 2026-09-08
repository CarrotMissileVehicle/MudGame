/**
 * @file BlacksmithPanel.cpp
 * @brief 铁匠铺面板实现。
 */
#include "BlacksmithPanel.h"

namespace mud::view
{
    void BlacksmithPanel::render(const BlacksmithView& view) const
    {
        r_.print("金币：" + std::to_string(view.gold));
        r_.print("[blacksmith] 铁匠铺——工具修复");
        r_.print("修复方式：金币（按损耗折算）或矿石（费用减半 + 消耗矿石）");
        for (const auto& t : view.tools)
        {
            std::string line = "  " + t.name + "：耐久 " + std::to_string(t.durability)
                               + "/" + std::to_string(t.max_durability);
            if (t.broken) line += " [已损坏]";
            line += "，修复需 " + t.repair_ore + " x" + std::to_string(t.repair_ore_needed)
                    + "（持有 " + std::to_string(t.repair_ore_held) + "）或金币 "
                    + std::to_string(t.repair_gold);
            r_.print(line);
        }
    }
} // namespace mud::view