#ifndef MUDGAME_NORMAL_FERTILIZER_H
#define MUDGAME_NORMAL_FERTILIZER_H


#include "Fertilizer.h"

// 普通肥料：剩余生长周期减半
class NormalFertilizer: public Fertilizer
{
public:
    NormalFertilizer();

    ~NormalFertilizer() override;
};


#endif //MUDGAME_NORMAL_FERTILIZER_H
