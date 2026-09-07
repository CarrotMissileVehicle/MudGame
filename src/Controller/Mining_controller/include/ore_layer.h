/**
 * @file ore_layer.h
 * @brief 矿区（层）数据类（Layer）。
 *
 * 管理矿区层级定义及其属性查询接口。
 */
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

/** @brief 矿区（层）数据查询类。 */
class Layer
{
public:
    /** @brief 默认构造：从数据目录加载矿区层级定义（JSON）。 */
    Layer();

    /** @brief 矿区照明需求类型。 */
    enum class LightingType {
        None,   // 不需要照明
        Torch,  // 火把
        Lantern // 灯笼
    };

    /** @brief 单个矿区的静态属性。 */
    struct MiningLayer {
        std::string name;       // 矿区名称
        size_t mining_level;    // 解锁所需采矿等级
        LightingType lighting;  // 照明需求
    };

    using miningLayerTable = std::unordered_map<std::string, MiningLayer>; // layer_id → MiningLayer

private:
    miningLayerTable layers_; // 矿区属性表

public:
    // layers
    /** @brief 查询矿区名称。 */
    std::string     get_layer_name(const std::string& layer_id) const;
    /** @brief 查询矿区解锁所需采矿等级。 */
    size_t          get_layer_level(const std::string& layer_id) const;
    /** @brief 查询矿区的照明需求类型。 */
    LightingType    get_lighting_type(const std::string& layer_id) const;
    /** @brief 判断矿区是否需要照明才能进入。 */
    bool            requires_lighting(const std::string& layer_id) const;
};