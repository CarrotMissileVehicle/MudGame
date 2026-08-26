#include "ore_layer.h"

// 获取矿区名称
std::string Layer::get_layer_name(const std::string& layer_id) const
{
    // TODO: 实现
}

// 获取矿区解锁所需采矿等级
size_t Layer::get_layer_level(const std::string& layer_id) const
{
    // TODO: 实现
}

// 获取矿区的照明需求类型
Layer::LightingType Layer::get_lighting_type(const std::string& layer_id) const
{
    // TODO: 实现
}

// 是否需要对矿区进行照明
bool Layer::requires_lighting(const std::string& layer_id) const
{
    // TODO: 实现
}