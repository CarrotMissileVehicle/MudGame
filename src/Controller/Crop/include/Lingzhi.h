#ifndef MUDGAME_LINGZHI_H
#define MUDGAME_LINGZHI_H


#include "Crop.h"

//灵芝
class Lingzhi: public Crop
{
public:
    Lingzhi();
    ~Lingzhi();

private:
    void grow();
    void harvest();
};


#endif //MUDGAME_LINGZHI_H
