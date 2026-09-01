#ifndef MUDGAME_FISH_H
#define MUDGAME_FISH_H

#include "Food.h"

class Fish: public Food
{
public:
    Fish(float probability, int fishExp, int satiationRecovery,
         int sellingPrice, int buyingPrice);

    ~Fish() override;

    float getProbability() const;
    int getFishExp() const;

    virtual void specialPurpose() {}

private:
    float probability;
    int fishExp;
};

#endif //MUDGAME_FISH_H
