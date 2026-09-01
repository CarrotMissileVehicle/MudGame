#ifndef MUDGAME_FARMLAND_H
#define MUDGAME_FARMLAND_H


#include "Crop.h"

class FarmLand
{
public:
    explicit FarmLand(Crop* crop = nullptr);

    ~FarmLand();

    bool isOccupied() const;
    bool isWatered() const;
    Crop* getCrop() const;
    int getGrowthStage() const;

    bool sow(Crop* seed);
    bool water();
    void fertilize(int speedUp);
    int harvest();
    void tickGrow(bool growFullSpeed);

private:
    bool occupied = false;          //是否占用
    bool watered = false;           //是否浇水
    Crop *crop = nullptr;           //所种作物
    int growthStage = 0;            //当前生长阶段
};


#endif //MUDGAME_FARMLAND_H
