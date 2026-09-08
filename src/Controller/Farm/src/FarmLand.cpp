#include "../include/farmland.h"

#include <algorithm>

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
    // DEF-008：按「周期/speedUp」直接补贴生长进度——播种后立即施肥同样有效
    // （等价于总周期缩短为 1/speedUp），补贴后钳制到周期上限防越界展示。
    // 合并远端的成熟保护：已成熟时施肥不再叠加。
    if (!occupied || speedUp <= 0) return;
    const int cycle = (crop != nullptr) ? crop->getGrowthCycle() : 0;
    if (cycle <= 0) return;
    if (growthStage >= cycle) return; // 已成熟，施肥不再叠加
    growthStage += std::max(1, cycle / speedUp);
    if (growthStage > cycle) growthStage = cycle;
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
    if (!watered) return; // 未浇水不生长（浇水由玩家手动或雨天自动进行）
    growthStage += growFullSpeed ? 2 : 1;
    if (growthStage > crop->getGrowthCycle()) // 钳制到生长周期上限，避免超出显示
        growthStage = crop->getGrowthCycle();
}
