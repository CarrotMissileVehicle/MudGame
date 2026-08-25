#ifndef MUDGAME_FERTILIZER_H
#define MUDGAME_FERTILIZER_H


#include "Object.h"

class Fertilizer: public Object
{
public:
    int price;
    int speedUp;

private:
    Fertilizer();
    ~Fertilizer();
};


#endif //MUDGAME_FERTILIZER_H