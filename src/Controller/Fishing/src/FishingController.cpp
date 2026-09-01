#include "../include/FishingController.h"

#include <cstdlib>

FishingController::FishingController(const std::vector<Fish*>& fishPool)
        : fishPool(fishPool) {}

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
