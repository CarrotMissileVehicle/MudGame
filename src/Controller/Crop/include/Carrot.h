#ifndef MUDGAME_CARROT_H
#define MUDGAME_CARROT_H


#include "Crop.h"

//胡萝卜
class Carrot: public Crop
{
public:
    Carrot();

    ~Carrot() override;

    void grow() override;
    void harvest() override;
};


#endif //MUDGAME_CARROT_H
