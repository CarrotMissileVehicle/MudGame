#ifndef MUDGAME_CABBAGE_H
#define MUDGAME_CABBAGE_H


#include "Crop.h"

class Cabbage: public Crop
{
public:
    Cabbage();
    ~Cabbage();

private:
    void grow();
    void harvest();
};


#endif //MUDGAME_CABBAGE_H