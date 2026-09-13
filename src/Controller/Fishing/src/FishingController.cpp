#include "../include/FishingController.h"

#include <random>

namespace
{
    // DEF-012：thread_local 引擎收敛全局 rand（多线程调用安全）。
    std::mt19937& rng()
    {
        static thread_local std::mt19937 gen(std::random_device{}());
        return gen;
    }
}

FishingController::FishingController(const std::vector<Fish*>& fishPool, float catchRate)
        : fishPool(fishPool), catchRate(catchRate), baseCatchRate(catchRate), isDaytime(true) {}

FishingController::~FishingController() {}

Fish* FishingController::rollFish() const {
    if (fishPool.empty()) return nullptr;

    float total = 0.0f;
    for (Fish* fish : fishPool) total += fish->getProbability();

    double roll = std::uniform_real_distribution<double>(0.0, 1.0)(rng()) * total;
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

    double roll = std::uniform_real_distribution<double>(0.0, 1.0)(rng());
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
