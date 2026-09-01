#include "../include/Crop.h"

Crop::Crop(int growthCycle, int yield, int farmExp, int unlockLevel,
           int sellingPrice, int buyingPrice)
        : Object(sellingPrice, buyingPrice),
          growthCycle(growthCycle),
          yield(yield),
          farmExp(farmExp),
          unlockLevel(unlockLevel),
          isUnLocked(unlockLevel <= 1) {}

Crop::~Crop() {}

int Crop::getGrowthCycle() const { return growthCycle; }

int Crop::getYield() const { return yield; }

int Crop::getFarmExp() const { return farmExp; }

int Crop::getUnlockLevel() const { return unlockLevel; }

bool Crop::isUnlocked() const { return isUnLocked; }

void Crop::unlock() { isUnLocked = true; }
