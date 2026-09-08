#include "../include/FarmingController.h"

FarmingController::FarmingController(Farm* farm)
        : farm(farm) {}

FarmingController::~FarmingController() {}

bool FarmingController::sow(std::size_t index, Crop* seed) {
    if (farm == nullptr) return false;
    return farm->getFarmland(index).sow(seed);
}

bool FarmingController::water(std::size_t index) {
    if (farm == nullptr) return false;
    return farm->getFarmland(index).water();
}

bool FarmingController::waterAll() {
    if (farm == nullptr) return false;
    bool changed = false;
    for (std::size_t i = 0; i < farm->size(); ++i) {
        if (farm->getFarmland(i).water()) changed = true;
    }
    return changed;
}

bool FarmingController::fertilize(std::size_t index, Fertilizer* fertilizer) {
    if (farm == nullptr || fertilizer == nullptr) return false;
    auto& land = farm->getFarmland(index);
    if (!land.isOccupied()) return false; // 空地不可施肥
    land.fertilize(fertilizer->getSpeedUp());
    return true;
}

int FarmingController::harvest(std::size_t index) {
    if (farm == nullptr) return 0;
    return farm->getFarmland(index).harvest();
}

void FarmingController::tick(const mud::time::GameDateTime& time) {
    if (farm == nullptr) return;
    // 06:00 - 18:00 为白天，作物全速生长；夜间减半
    bool isDaytime = (time.hour >= 6 && time.hour < 18);
    farm->tickAll(isDaytime);
}

std::size_t FarmingController::farmSize() const {
    return farm != nullptr ? farm->size() : 0;
}
