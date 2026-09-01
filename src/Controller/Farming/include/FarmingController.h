#ifndef MUDGAME_FARMING_CONTROLLER_H
#define MUDGAME_FARMING_CONTROLLER_H


#include <cstddef>
#include "Farm.h"
#include "Crop.h"
#include "Fertilizer.h"

class FarmingController
{
public:
    explicit FarmingController(Farm* farm);

    ~FarmingController();

    bool sow(std::size_t index, Crop* seed);
    bool water(std::size_t index);
    bool waterAll();
    bool fertilize(std::size_t index, Fertilizer* fertilizer);
    int harvest(std::size_t index);
    void tick(bool isDaytime);
    std::size_t farmSize() const;

private:
    Farm* farm;
};


#endif //MUDGAME_FARMING_CONTROLLER_H
