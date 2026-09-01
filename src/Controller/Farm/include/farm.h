#ifndef MUDGAME_FARM_H
#define MUDGAME_FARM_H


#include <vector>
#include "FarmLand.h"

class Farm
{
public:
    explicit Farm(const std::vector<FarmLand>& farmlands);

    ~Farm();

    size_t size() const;
    FarmLand& getFarmland(size_t index);
    const FarmLand& getFarmland(size_t index) const;
    void add(const FarmLand& farmland);
    void tickAll(bool growFullSpeed);

private:
    std::vector<FarmLand> farmlands;
};


#endif //MUDGAME_FARM_H
