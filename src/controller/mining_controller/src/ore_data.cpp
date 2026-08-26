#include "ore_data.h"

// 获取矿石名称
const std::string& Ore::get_ore_name(const std::string& ore_id) const
{
    // TODO: 实现
}

// 获取矿石用途说明
const std::string& Ore::get_ore_usage(const std::string& ore_id) const
{
    // TODO: 实现
}

// 获取矿石价格
std::size_t Ore::get_ore_price(const std::string& ore_id) const
{
    // TODO: 实现
}

// 获取矿石开采经验
std::size_t Ore::get_ore_mining_exp(const std::string& ore_id) const
{
    // TODO: 实现
}

// 获取矿石解锁所需采矿等级
std::size_t Ore::get_ore_mining_level(const std::string& ore_id) const
{
    // TODO: 实现
}

// 获取某层内某矿石的生成概率
double Ore::get_ore_rate(const std::string& layer_id,
                        const std::string& ore_id) const
{
    // TODO: 实现
}

// 获取某层内的矿石分布表
const Ore::spawnRates& Ore::get_ore_distribution(const std::string& layer_id) const
{
    // TODO: 实现
}