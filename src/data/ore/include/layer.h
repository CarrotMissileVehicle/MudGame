#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class Layer
{
    enum class LightingType {
        None,   // 不需要照明
        Torch,  // 火把
        Lantern // 灯笼
    };

    struct MiningLayer {
        std::string name;       // 矿区名称
        size_t mining_level;    // 解锁所需采矿等级
        LightingType lighting;  // 照明需求
    };

    using miningLayerTable = std::unordered_map<std::string, MiningLayer>;

private:
    miningLayerTable layers_;

public:
    // layers
    std::string get_layer_name();

    size_t get_layer_level();
};