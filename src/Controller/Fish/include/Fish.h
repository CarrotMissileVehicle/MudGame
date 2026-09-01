#ifndef MUDGAME_FISH_H
#define MUDGAME_FISH_H

#include "Object.h"

class Fish: public Object
{
public:
    Fish(float probability, int fishExp, int sellingPrice, int buyingPrice);

    ~Fish() override;

    float getProbability() const;
    int getFishExp() const;

    virtual void specialPurpose() {}

private:
    float probability;
    int fishExp;
};

#endif //MUDGAME_FISH_H
