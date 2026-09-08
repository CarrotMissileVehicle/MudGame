/**
 * @file ore_data.h
 * @brief 矿石数据表（Ore）。
 *
 * 管理与矿石定义、各矿区产出权重分布相关的查询接口。
 *
 * 依赖：object 基类。
 */
#pragma once

#include "Object.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

/** @brief 矿石数据查询类：提供矿石属性及各层产出分布查询。 */
class Ore : public Object
{
public:
    /** @brief 默认构造：从数据目录加载矿石属性与各层产出分布（JSON）。 */
    Ore();

    /** @brief 一种矿石的静态属性。 */
    struct OreData
    {
        std::string name;          // 矿石名称
        size_t mining_level;       // 所需采矿等级
        size_t price;              // 售价
        size_t mining_exp;         // 单次采矿经验
        std::string usage;         // 用途说明
    };

private:
    using spawnRates = std::unordered_map<std::string, double>;            // ore_id → 权重
    using spawnRateTable = std::unordered_map<std::string, spawnRates>;    // layer_id → spawnRates
    using oreTable = std::unordered_map<std::string, OreData>;             // ore_id → OreData

    spawnRateTable rates_; // 各矿区矿石产出权重表
    oreTable ores_;        // 矿石属性表

public:
    /** @brief 查询矿石名称。 */
    const std::string& get_ore_name(const std::string& ore_id) const;

    /** @brief 查询矿石用途。 */
    const std::string& get_ore_usage(const std::string& ore_id) const;

    /** @brief 查询矿石售价。 */
    std::size_t get_ore_price(const std::string& ore_id) const;

    /** @brief 查询矿石单次采矿经验。 */
    std::size_t get_ore_mining_exp(const std::string& ore_id) const;

    /** @brief 查询矿石所需采矿等级。 */
    std::size_t get_ore_mining_level(const std::string& ore_id) const;

    /** @brief 查询指定矿区中某矿石的产出权重。 */
    double get_ore_rate(const std::string& layer_id,
                        const std::string& ore_id) const;

    /** @brief 查询指定矿区的完整产出分布权重表。 */
    const spawnRates& get_ore_distribution(const std::string& layer_id) const;
};