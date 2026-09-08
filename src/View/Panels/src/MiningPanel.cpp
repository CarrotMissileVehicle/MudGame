/**
 * @file MiningPanel.cpp
 * @brief 采矿面板实现。
 */
#include "MiningPanel.h"

#include "view_primitives.h"

namespace mud::view
{
    void MiningPanel::render_status(const MiningView& mining) const
    {
        if (mining.is_mining)
        {
            r_.print("状态：采矿中（层 " + std::to_string(mining.layer) + "，自 " +
                     format_time(mining.start_time) + " 开始）");
        }
        else
        {
            r_.print("状态：空闲");
        }
        r_.print("采矿等级：" + std::to_string(mining.mining_level));
    }

    void MiningPanel::render_produce(const std::vector<MiningEventView>& results) const
    {
        for (const auto& ev : results)
            r_.print("  获得 " + ev.ore_name + " x" + std::to_string(ev.quantity) + " (+" +
                     std::to_string(ev.experience) + " 采矿经验)");
    }
} // namespace mud::view