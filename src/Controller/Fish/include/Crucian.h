#ifndef MUDGAME_CRUCIAN_H
#define MUDGAME_CRUCIAN_H

#include "Fish.h"

//小鲫鱼
class Crucian: public Fish
{
public:
    Crucian();

    ~Crucian() override;

    void specialPurpose() override;
};

#endif //MUDGAME_CRUCIAN_H
