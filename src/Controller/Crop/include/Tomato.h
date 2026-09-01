#ifndef MUDGAME_TOMATO_H
#define MUDGAME_TOMATO_H


#include "Crop.h"

//西红柿
class Tomato: public Crop
{
public:
    Tomato();

    ~Tomato() override;

    void grow() override;
    void harvest() override;
};


#endif //MUDGAME_TOMATO_H
