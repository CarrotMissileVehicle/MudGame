#ifndef MUDGAME_FARM_H
#define MUDGAME_FARM_H


#include <vector>
#include "farmland.h"

class Farm
{
public:
    Farm();
    ~Farm();

private:
    std::vector<FarmLand> farmlands;

    void add();
};


#endif //MUDGAME_FARM_H