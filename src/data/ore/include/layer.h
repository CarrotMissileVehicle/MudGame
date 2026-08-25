#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class Layer
{
    struct MiningLayer {
        std::string layer_name;

        size_t mining_level;
    };

    using miningLayerTable = std::unordered_map<std::string, MiningLayer>;

private:
    miningLayerTable layers_;

public:
    // layers
    std::string get_layer_name();

    size_t get_layer_level();
};