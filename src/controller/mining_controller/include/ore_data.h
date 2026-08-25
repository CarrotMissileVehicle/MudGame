#pragma once

#include "object.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class Ore : public object
{
public:
    struct OreData
    {
        std::string name;

        size_t mining_level;
        size_t price;
        size_t mining_exp;

        std::string usage;
    };

private:
    using spawnRates = std::unordered_map<std::string, double>;
    using spawnRateTable = std::unordered_map<std::string, spawnRates>;
    using oreTable = std::unordered_map<std::string, OreData>;

    spawnRateTable rates_;
    oreTable ores_;

public:
    const std::string& get_ore_name(const std::string& ore_id) const;

    const std::string& get_ore_usage(const std::string& ore_id) const;

    std::size_t get_ore_price(const std::string& ore_id) const;

    std::size_t get_ore_mining_exp(const std::string& ore_id) const;

    std::size_t get_ore_mining_level(const std::string& ore_id) const;

    double get_ore_rate(const std::string& layer_id,
                        const std::string& ore_id) const;

    const spawnRates& get_ore_distribution(const std::string& layer_id) const;
};