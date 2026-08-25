#ifndef MUDGAME_CROP_H
#define MUDGAME_CROP_H


#include "Object.h"

class Crop: public Object
{
public:
    Crop();
    ~Crop();

private:
    int seedPrice;              //种子价格
    int growthCycle;            //生长周期
    int yield;                  //收获数量
    int producePrice;           //售卖价格
    int farmExp;                //农业经验
    int unLockCondition;        //解锁等级
    bool isUnLocked = false;    //是否解锁
    void grow();
    void harvest();
};


#endif //MUDGAME_CROP_H
