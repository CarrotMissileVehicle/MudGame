/**
 * @file StatusPanel.cpp
 * @brief 玩家状态面板实现（自原 StatusView 迁移，改用 Renderer 输出）。
 */
#include "StatusPanel.h"

namespace mud::view
{
    void StatusPanel::render(const PlayerStatus& status) const
    {
        r_.print("\n========== Player Status ==========");
        r_.print("Position:   " + GetPositionName(status.position));
        r_.print("State:      " + GetStateName(status.state));
        r_.print("Satiety:    " + std::to_string(status.satiety) + " / " +
                 std::to_string(status.maxSatiety));
        r_.print("-----------------------------------");
        r_.print("Exp - Farming: " + std::to_string(status.farmingExp));
        r_.print("Exp - Fishing: " + std::to_string(status.fishExp));
        r_.print("Exp - Mining:  " + std::to_string(status.mineExp));
        r_.print("-----------------------------------");
        r_.print("Bag (" + std::to_string(status.bagItems.size()) + " items):");
        if (status.bagItems.empty())
        {
            r_.print("  (empty)");
        }
        else
        {
            for (std::size_t i = 0; i < status.bagItems.size(); ++i)
                r_.print("  " + std::to_string(i + 1) + ". " + status.bagItems[i]);
        }
        r_.print("===================================\n");
    }

    std::string StatusPanel::GetPositionName(PositionCode code)
    {
        switch (code)
        {
            case AtHome:     return "Home";
            case AtFarmland: return "Farmland";
            case AtCoast:    return "Coast";
            case AtMine:     return "Mine";
            case AtTown:     return "Town";
            default:         return "Unknown";
        }
    }

    std::string StatusPanel::GetStateName(StateCode code)
    {
        switch (code)
        {
            case StateCode::Waiting:     return "Waiting";
            case StateCode::Moving:      return "Moving";
            case StateCode::Watering:    return "Watering";
            case StateCode::Seeding:     return "Seeding";
            case StateCode::Fertilizing: return "Fertilizing";
            case StateCode::Sleeping:    return "Sleeping";
            case StateCode::Shopping:    return "Shopping";
            case StateCode::Repairing:   return "Repairing";
            case StateCode::Fishing:     return "Fishing";
            case StateCode::Mining:      return "Mining";
            default:                     return "Unknown";
        }
    }
} // namespace mud::view