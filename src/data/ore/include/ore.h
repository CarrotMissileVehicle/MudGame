#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class Ore
{
    struct ore
    {
        std::string name;

        size_t mining_level;
        size_t price;
        size_t mining_exp;

        std::string usage;
    };

    using spawnRates        = std::unordered_map<std::string, double>;
    using spawnRateTable    = std::unordered_map<std::string, spawnRates>;
    using oreTable          = std::unordered_map<std::string, Ore>;

private:
    spawnRateTable rates_;
    oreTable ores_;

public:
    // ores
    std::string get_ore_name();
    std::string get_ore_usage();

    double get_ore_price();
    double get_ore_miningExp();

    size_t get_ore_miningLevel();

    // rates
    std::string get_ore_distribution();

    double get_ore_rates(std::string layer_name);
};