#ifndef MUDGAME_CROP_H
#define MUDGAME_CROP_H


#include "Food.h"

class Crop: public Food
{
public:
    Crop(int growthCycle, int yield, int farmExp, int unlockLevel,
         int satiationRecovery, int sellingPrice, int buyingPrice);

    ~Crop() override;

    int getGrowthCycle() const;
    int getYield() const;
    int getFarmExp() const;
    int getUnlockLevel() const;
    bool isUnlocked() const;
    void unlock();

private:
    int growthCycle;            //生长周期
    int yield;                  //收获数量
    int farmExp;                //农业经验
    int unlockLevel;            //解锁等级
    bool isUnLocked = false;    //是否解锁
};


#endif //MUDGAME_CROP_H
