//
// Created by opencode on 2026/9/1.
//

#include "../include/StatusView.h"
#include <iostream>
#include <iomanip>

void StatusView::ShowStatus(const PlayerStatus& status) {
    std::cout << "\n========== Player Status ==========\n";
    std::cout << "Position:   " << GetPositionName(status.position) << "\n";
    std::cout << "State:      " << GetStateName(status.state) << "\n";
    std::cout << "Satiety:    " << status.satiety << " / " << status.maxSatiety << "\n";
    std::cout << "-----------------------------------\n";
    std::cout << "Exp - Farming: " << status.farmingExp << "\n";
    std::cout << "Exp - Fishing: " << status.fishExp << "\n";
    std::cout << "Exp - Mining:  " << status.mineExp << "\n";
    std::cout << "-----------------------------------\n";
    std::cout << "Bag (" << status.bagItems.size() << " items):\n";
    if (status.bagItems.empty()) {
        std::cout << "  (empty)\n";
    } else {
        for (size_t i = 0; i < status.bagItems.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << status.bagItems[i] << "\n";
        }
    }
    std::cout << "===================================\n\n";
}

std::string StatusView::GetPositionName(PositionCode code) const {
    switch (code) {
        case AtHome:      return "Home";
        case AtFarmland:  return "Farmland";
        case AtCoast:     return "Coast";
        case AtMine:      return "Mine";
        case AtTown:      return "Town";
        default:        return "Unknown";
    }
}

std::string StatusView::GetStateName(StateCode code) const {
    switch (code) {
        case Waiting:       return "Waiting";
        case Moving:        return "Moving";
        case Watering:      return "Watering";
        case Seeding:       return "Seeding";
        case Fertilizing:   return "Fertilizing";
        case Sleeping:      return "Sleeping";
        case Shopping:      return "Shopping";
        case Repairing:     return "Repairing";
        case Fishing:       return "Fishing";
        case Mining:        return "Mining";
        default:            return "Unknown";
    }
}
