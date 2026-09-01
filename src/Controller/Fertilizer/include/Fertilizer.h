#ifndef MUDGAME_FERTILIZER_H
#define MUDGAME_FERTILIZER_H


#include "Object.h"

class Fertilizer: public Object
{
public:
    int getSpeedUp() const;

protected:
    Fertilizer(int speedUp, int sellingPrice, int buyingPrice);

    ~Fertilizer() override;

private:
    int speedUp;
};


#endif //MUDGAME_FERTILIZER_H
