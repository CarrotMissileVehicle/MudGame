#include "../include/FarmLand.h"

FarmLand::FarmLand(Crop* crop)
        : crop(crop) {
    occupied = (crop != nullptr);
    watered = false;
    growthStage = 0;
}

FarmLand::~FarmLand() {}

bool FarmLand::isOccupied() const { return occupied; }

bool FarmLand::isWatered() const { return watered; }

Crop* FarmLand::getCrop() const { return crop; }

int FarmLand::getGrowthStage() const { return growthStage; }

bool FarmLand::sow(Crop* seed) {
    if (occupied || seed == nullptr) return false;
    crop = seed;
    occupied = true;
    watered = false;
    growthStage = 0;
    return true;
}

bool FarmLand::water() {
    if (!occupied) return false;
    watered = true;
    return true;
}

void FarmLand::fertilize(int speedUp) {
    // speedUp: 剩余生长周期的加速除数（2 减半，3 减为 1/3）
    if (speedUp > 0) growthStage += growthStage / speedUp;
}

int FarmLand::harvest() {
    if (!occupied || crop == nullptr) return 0;
    if (growthStage < crop->getGrowthCycle()) return 0;
    int result = crop->getYield();
    crop = nullptr;
    occupied = false;
    watered = false;
    growthStage = 0;
    return result;
}

void FarmLand::tickGrow(bool growFullSpeed) {
    if (!occupied) return;
    growthStage += growFullSpeed ? 2 : 1;
    watered = true;
}
