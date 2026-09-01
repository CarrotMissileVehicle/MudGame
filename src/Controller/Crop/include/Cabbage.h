#ifndef MUDGAME_CABBAGE_H
#define MUDGAME_CABBAGE_H


#include "Crop.h"

//小白菜
class Cabbage: public Crop
{
public:
    Cabbage();

    ~Cabbage() override;

    void grow() override;
    void harvest() override;
};


#endif //MUDGAME_CABBAGE_H
