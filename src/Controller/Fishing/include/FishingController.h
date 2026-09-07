#ifndef MUDGAME_FISHING_CONTROLLER_H
#define MUDGAME_FISHING_CONTROLLER_H


#include <cstddef>
#include <vector>
#include "Fish.h"
#include "Time.h"

class FishingController
{
public:
    explicit FishingController(const std::vector<Fish*>& fishPool, float catchRate = 0.3f);

    ~FishingController();

    Fish* rollFish() const;
    Fish* tickFish();
    // 按游戏时间推进：白天(06-18点)用基础成功率，夜间减半
    void tick(const Time& time);
    void setCatchRate(float catchRate);
    std::size_t poolSize() const;

private:
    std::vector<Fish*> fishPool;
    float catchRate;
    float baseCatchRate;   // 基础成功率（白天基准）
    bool isDaytime = true; // 是否白天，影响成功率
};


#endif //MUDGAME_FISHING_CONTROLLER_H
