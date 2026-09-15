/**
 * @file ore_layer.cpp
 * @brief 矿区（层）数据查询实现。
 *
 * 通过 pjh_json 从游戏数据目录解析 mining_layers.json，
 * 预填充矿区层级定义，提供查询。
 */
#include "ore_layer.h"

#include "pjh_json.hpp"

#include <stdexcept>
#include <string>

namespace
{
    inline const std::string& data_dir()
    {
        static const std::string dir = MUDGAME_DATA_DIR;
        return dir;
    }
}

Layer::Layer()
{
    using pjh::json::parse_file;

    const auto path = data_dir() + "/Ore/mining_layers.json";
    auto doc = parse_file(path);

    for (const auto& [key, node] : doc.root().as_object())
    {
        const auto id = std::string(std::string_view(key));
        const auto& l = node.as_object();

        MiningLayer ml;
        ml.name         = std::string(l.at("name").as_string());
        ml.mining_level = static_cast<std::size_t>(l.at("mining_level").as_int());

        const std::string lighting(l.at("lighting").as_string());
        // H12：显式枚举合法光照类型，未知值直接报错而非静默映射为 Lantern，
        // 避免配置笔误被掩盖成错误的游戏行为
        if (lighting == "none") {
            ml.lighting = LightingType::None;
        } else if (lighting == "torch") {
            ml.lighting = LightingType::Torch;
        } else if (lighting == "lantern") {
            ml.lighting = LightingType::Lantern;
        } else {
            throw std::invalid_argument("无效的光照类型: " + lighting + "（layer: " + id + "）");
        }

        layers_.emplace(id, std::move(ml));
    }
}

std::string Layer::get_layer_name(const std::string& layer_id) const
{
    const auto it = layers_.find(layer_id);
    if (it == layers_.end()) throw std::out_of_range("unknown layer: " + layer_id);
    return it->second.name;
}

size_t Layer::get_layer_level(const std::string& layer_id) const
{
    const auto it = layers_.find(layer_id);
    if (it == layers_.end()) throw std::out_of_range("unknown layer: " + layer_id);
    return it->second.mining_level;
}

Layer::LightingType Layer::get_lighting_type(const std::string& layer_id) const
{
    const auto it = layers_.find(layer_id);
    if (it == layers_.end()) throw std::out_of_range("unknown layer: " + layer_id);
    return it->second.lighting;
}

bool Layer::requires_lighting(const std::string& layer_id) const
{
    const auto it = layers_.find(layer_id);
    if (it == layers_.end()) throw std::out_of_range("unknown layer: " + layer_id);
    return it->second.lighting != LightingType::None;
}