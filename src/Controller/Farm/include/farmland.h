#ifndef MUDGAME_FARMLAND_H
#define MUDGAME_FARMLAND_H


#include "Crop.h"

class FarmLand
{
public:
    // 借用语义：crop 仅被保存指针、不拥有、不释放；调用方必须保证
    // 传入对象在 FarmLand 存活期间一直有效（如 GameSession 中的原型成员）。
    explicit FarmLand(Crop* crop = nullptr);

    ~FarmLand();

    bool isOccupied() const;
    bool isWatered() const;
    // 借用语义：返回非拥有指针，调用方不得 delete。
    Crop* getCrop() const;
    int getGrowthStage() const;

    // 借用语义：seed 仅被保存指针、不拥有、不释放；调用方必须保证其
    // 生命周期覆盖整个播种期（传入临时对象会导致后续 tick/harvest 悬垂）。
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
