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
    farm->getFarmland(index).fertilize(fertilizer->getSpeedUp());
    return true;
}

int FarmingController::harvest(std::size_t index) {
    if (farm == nullptr) return 0;
    return farm->getFarmland(index).harvest();
}

void FarmingController::tick(bool isDaytime) {
    if (farm != nullptr) farm->tickAll(isDaytime);
}

std::size_t FarmingController::farmSize() const {
    return farm != nullptr ? farm->size() : 0;
}
