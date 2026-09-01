#ifndef MUDGAME_FISHING_CONTROLLER_H
#define MUDGAME_FISHING_CONTROLLER_H


#include <cstddef>
#include <vector>
#include "Fish.h"

class FishingController
{
public:
    explicit FishingController(const std::vector<Fish*>& fishPool);

    ~FishingController();

    Fish* rollFish() const;
    std::size_t poolSize() const;

private:
    std::vector<Fish*> fishPool;
};


#endif //MUDGAME_FISHING_CONTROLLER_H
