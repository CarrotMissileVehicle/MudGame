#ifndef MARKET_H
#define MARKET_H

#include <vector>
#include "Object.h"

class Market
{
public:
    Market();
    ~Market();

    void buy(Object *obj);
    void sold(Object *obj);

    void onNewDay(int week, int day);

private:
    std::vector<Object> objects;

};


#endif