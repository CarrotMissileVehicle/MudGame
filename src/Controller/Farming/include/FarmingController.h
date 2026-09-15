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

    // 借用语义：seed 仅被地块保存指针、不拥有、不释放；调用方必须保证其
    // 生命周期覆盖整个播种期（如 GameSession 中的作物原型成员）。
    bool sow(std::size_t index, Crop* seed);
    bool water(std::size_t index);
    bool waterAll();
    // 借用语义：fertilizer 仅即时读取（getSpeedUp()），调用方保证调用期间有效即可。
    bool fertilize(std::size_t index, Fertilizer* fertilizer);
    int harvest(std::size_t index);
    // 按游戏时间推进作物生长：白天(06-18点)全速，夜间减半
    void tick(const mud::time::GameDateTime& time);
    std::size_t farmSize() const;

private:
    Farm* farm;
};


#endif //MUDGAME_FARMING_CONTROLLER_H
