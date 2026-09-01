#ifndef MUDGAME_FISHING_CONTROLLER_H
#define MUDGAME_FISHING_CONTROLLER_H


#include <cstddef>
#include <vector>
#include "Fish.h"

class FishingController
{
public:
    explicit FishingController(const std::vector<Fish*>& fishPool, float catchRate = 0.3f);

    ~FishingController();

    Fish* rollFish() const;
    Fish* tickFish();
    void setCatchRate(float catchRate);
    std::size_t poolSize() const;

private:
    std::vector<Fish*> fishPool;
    float catchRate;
};


#endif //MUDGAME_FISHING_CONTROLLER_H
