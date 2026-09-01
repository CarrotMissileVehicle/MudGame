#include "../include/Farm.h"

Farm::Farm(const std::vector<FarmLand>& farmlands)
        : farmlands(farmlands) {}

Farm::~Farm() {}

size_t Farm::size() const { return farmlands.size(); }

FarmLand& Farm::getFarmland(size_t index) { return farmlands.at(index); }

const FarmLand& Farm::getFarmland(size_t index) const { return farmlands.at(index); }

void Farm::add(const FarmLand& farmland) { farmlands.push_back(farmland); }

void Farm::tickAll(bool growFullSpeed) {
    for (auto& farmland : farmlands) {
        farmland.tickGrow(growFullSpeed);
    }
}
