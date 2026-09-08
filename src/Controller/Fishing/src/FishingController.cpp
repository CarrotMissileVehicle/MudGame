#include "../include/FishingController.h"

#include <cstdlib>

FishingController::FishingController(const std::vector<Fish*>& fishPool, float catchRate)
        : fishPool(fishPool), catchRate(catchRate), baseCatchRate(catchRate), isDaytime(true) {}

FishingController::~FishingController() {}

Fish* FishingController::rollFish() const {
    if (fishPool.empty()) return nullptr;

    float total = 0.0f;
    for (Fish* fish : fishPool) total += fish->getProbability();

    double roll = static_cast<double>(std::rand()) / RAND_MAX * total;
    float accumulated = 0.0f;
    for (Fish* fish : fishPool) {
        accumulated += fish->getProbability();
        if (roll < accumulated) return fish;
    }
    return fishPool.back();
}

std::size_t FishingController::poolSize() const {
    return fishPool.size();
}

Fish* FishingController::tickFish() {
    if (fishPool.empty()) return nullptr;

    double roll = static_cast<double>(std::rand()) / RAND_MAX;
    if (roll >= catchRate) return nullptr;

    return rollFish();
}

void FishingController::setCatchRate(float catchRate) {
    this->baseCatchRate = catchRate;
    this->catchRate = isDaytime ? catchRate : catchRate * 0.5f;
}

void FishingController::tick(const mud::time::GameDateTime& time) {
    // 06:00 - 18:00 为白天，全速；夜间成功率减半
    isDaytime = (time.hour >= 6 && time.hour < 18);
    catchRate = isDaytime ? baseCatchRate : baseCatchRate * 0.5f;
}
