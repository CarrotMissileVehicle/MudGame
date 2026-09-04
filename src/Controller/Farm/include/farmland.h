#ifndef MUDGAME_FARMLAND_H
#define MUDGAME_FARMLAND_H


#include "Crop.h"

class FarmLand
{
private:
    bool isOccupied = false;        //是否占用
    bool isWatered = false;         //是否浇水

    Crop *crop;                     //所种作物

    void sow();
    void water();
    void fertilize();
    void harvest();

public:
    FarmLand();
    ~FarmLand();
};


#endif //MUDGAME_FARMLAND_H