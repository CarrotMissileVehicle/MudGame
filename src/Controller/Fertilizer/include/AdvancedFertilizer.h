#ifndef MUDGAME_ADVANCED_FERTILIZER_H
#define MUDGAME_ADVANCED_FERTILIZER_H


#include "Fertilizer.h"

// 高级肥料：剩余生长周期减为 1/3
class AdvancedFertilizer: public Fertilizer
{
public:
    AdvancedFertilizer();

    ~AdvancedFertilizer() override;
};


#endif //MUDGAME_ADVANCED_FERTILIZER_H
