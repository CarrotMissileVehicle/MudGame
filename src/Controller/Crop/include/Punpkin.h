#ifndef MUDGAME_PUMPKIN_H
#define MUDGAME_PUMPKIN_H


#include "Crop.h"

//南瓜
class Pumpkin: public Crop
{
public:
    Pumpkin();

    ~Pumpkin() override;

    void grow() override;
    void harvest() override;
};


#endif //MUDGAME_PUMPKIN_H
