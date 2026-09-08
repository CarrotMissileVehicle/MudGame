#ifndef MUDGAME_FARM_H
#define MUDGAME_FARM_H


#include <vector>
#include "farmland.h"

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

    // 对已占用的地块统一浇水（天气系统在雨天自动调用）
    void autoWater() {
        for (auto& farmland : farmlands) {
            if (farmland.isOccupied()) farmland.water();
        }
    }

private:
    std::vector<FarmLand> farmlands;
};


#endif //MUDGAME_FARM_H
