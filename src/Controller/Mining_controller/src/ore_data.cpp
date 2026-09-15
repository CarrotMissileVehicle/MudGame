/**
 * @file ore_data.cpp
 * @brief 矿石数据表查询实现。
 *
 * 通过 pjh_json 从游戏数据目录解析 ore.json 与 spawn_rates.json，
 * 预填充矿石属性表与各层产出分布，提供查询。
 */
#include "ore_data.h"

#include "pjh_json.hpp"

#include <stdexcept>
#include <string>

#include <filesystem>
#ifdef _WIN32
    #include <windows.h>
#endif

// 数据目录定位：优先使用 exe 同级的 Data/（发布包随行数据，跨机分发可读），
// 找不到时回落编译期注入的 MUDGAME_DATA_DIR（单元测试场景依赖后者）。
namespace
{
    inline std::string data_dir()
    {
#ifdef _WIN32
        wchar_t buf[4096];
        const DWORD len = GetModuleFileNameW(nullptr, buf, 4096);
        if (len > 0 && len < 4096)
        {
            std::filesystem::path exe(buf);
            const std::filesystem::path rel = exe.parent_path() / "Data";
            if (std::filesystem::exists(rel / "Ore"))
                return rel.string();
        }
#endif
        return MUDGAME_DATA_DIR;
    }
}

Ore::Ore()
{
    using pjh::json::parse_file;
    namespace json = pjh::json;

    // 矿石属性表：ore.json
    const auto ore_path = data_dir() + "/Ore/ore.json";
    auto ore_doc = parse_file(ore_path);
    for (const auto& [key, node] : ore_doc.root().as_object())
    {
        const auto id = std::string(std::string_view(key));
        const auto& o = node.as_object();
        OreData d;
        d.name        = std::string(o.at("name").as_string());
        d.mining_level = static_cast<std::size_t>(o.at("mining_level").as_int());
        d.price       = static_cast<std::size_t>(o.at("price").as_int());
        d.mining_exp  = static_cast<std::size_t>(o.at("mining_exp").as_int());
        d.usage       = std::string(o.at("usage").as_string());
        ores_.emplace(id, std::move(d));
    }

    // 各层产出分布：spawn_rates.json
    const auto rate_path = data_dir() + "/Ore/spawn_rates.json";
    auto rate_doc = parse_file(rate_path);
    for (const auto& [layer, lnode] : rate_doc.root().as_object())
    {
        const auto lid = std::string(std::string_view(layer));
        spawnRates layer_rates;
        for (const auto& [ore, rate] : lnode.as_object())
            layer_rates.emplace(
                std::string(std::string_view(ore)), rate.as_float());
        rates_.emplace(lid, std::move(layer_rates));
    }
}

const std::string& Ore::get_ore_name(const std::string& ore_id) const
{
    const auto it = ores_.find(ore_id);
    if (it == ores_.end()) throw std::out_of_range("unknown ore: " + ore_id);
    return it->second.name;
}

const std::string& Ore::get_ore_usage(const std::string& ore_id) const
{
    const auto it = ores_.find(ore_id);
    if (it == ores_.end()) throw std::out_of_range("unknown ore: " + ore_id);
    return it->second.usage;
}

std::size_t Ore::get_ore_price(const std::string& ore_id) const
{
    const auto it = ores_.find(ore_id);
    if (it == ores_.end()) throw std::out_of_range("unknown ore: " + ore_id);
    return it->second.price;
}

std::size_t Ore::get_ore_mining_exp(const std::string& ore_id) const
{
    const auto it = ores_.find(ore_id);
    if (it == ores_.end()) throw std::out_of_range("unknown ore: " + ore_id);
    return it->second.mining_exp;
}

std::size_t Ore::get_ore_mining_level(const std::string& ore_id) const
{
    const auto it = ores_.find(ore_id);
    if (it == ores_.end()) throw std::out_of_range("unknown ore: " + ore_id);
    return it->second.mining_level;
}

double Ore::get_ore_rate(
    const std::string& layer_id,
    const std::string& ore_id) const
{
    const auto lt = rates_.find(layer_id);
    if (lt == rates_.end()) return 0.0;
    const auto oit = lt->second.find(ore_id);
    if (oit == lt->second.end()) return 0.0;
    return oit->second;
}

const Ore::spawnRates& Ore::get_ore_distribution(
    const std::string& layer_id) const
{
    const auto it = rates_.find(layer_id);
    if (it == rates_.end()) throw std::out_of_range("unknown layer: " + layer_id);
    return it->second;
}