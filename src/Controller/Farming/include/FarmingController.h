#ifndef MUDGAME_FARMING_CONTROLLER_H
#define MUDGAME_FARMING_CONTROLLER_H


#include <cstddef>
#include "farm.h"
#include "Crop.h"
#include "Fertilizer.h"
#include "game_time.h"

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
    // 按游戏时间推进作物生长：白天(06-18点)全速，夜间减半
    void tick(const mud::time::GameDateTime& time);
    std::size_t farmSize() const;

private:
    Farm* farm;
};


#endif //MUDGAME_FARMING_CONTROLLER_H
