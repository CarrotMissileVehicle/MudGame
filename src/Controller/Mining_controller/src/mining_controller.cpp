/**
 * @file mining_controller.cpp
 * @brief 采矿控制器实现。
 *
 * 驱动采矿会话：启动校验、进度结算与产出计算。层/矿石数据来自
 * 内部 Ore/Layer 查询表（由 JSON 数据加载）。
 */
#include "mining_controller.h"

#include "mining_calculator.h"

#include <random>
#include <stdexcept>
#include <string>

namespace
{
    // size_t 层 id → JSON 层 key（与 mining::MiningLayer 枚举序一致）。
    bool layer_key_for(std::size_t layer_id, std::string& out)
    {
        static const char* const kLayerKeys[] = {
            "shallow", "middle", "deep", "crystal", "core"};
        if (layer_id >= 5)
            return false;
        out = kLayerKeys[layer_id];
        return true;
    }

    // 均匀随机数 [0,1)，线程局部引擎，const 查询方法中亦可使用。
    double rand_unit()
    {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(rng);
    }
}

MiningController::MiningController(
    const Ore::OreData& ore_data,
    const TimeService& time_service
) : ore_data_(ore_data), time_service_(time_service)
{
}

bool MiningController::start_mining(
    MiningState& state,
    std::size_t layer_id,
    const mining::MiningContext& context
)
{
    if (state.is_mining())
        return false;
    if (!can_enter_layer(layer_id, context))
        return false;

    state.start(layer_id, time_service_.now());
    return true;
}

std::vector<mining::MiningResult> MiningController::update(
    MiningState& state,
    const mining::MiningContext& context
)
{
    if (!state.is_mining())
        return {};

    const auto now = time_service_.now();
    const auto elapsed =
        now.total_minutes() - state.last_tick().total_minutes();

    const auto interval  = context.tool.interval;
    const auto count     = MiningCalculator::calculate_production_count(elapsed, interval);
    if (count == 0)
        return {};

    // 推进进度计时点至本次完整产出对齐的时间。
    const auto consumed = MiningCalculator::calculate_consumed_time(elapsed, interval);
    auto tick = state.last_tick();
    tick.advance(consumed);
    state.update_tick(tick);

    return produce(*state.layer_id(), count, context);
}

std::vector<mining::MiningResult> MiningController::stop_mining(
    MiningState& state,
    const mining::MiningContext& context
)
{
    if (!state.is_mining())
        return {};

    const auto layer = *state.layer_id();
    const auto now   = time_service_.now();
    const auto elapsed =
        now.total_minutes() - state.last_tick().total_minutes();

    const auto count =
        MiningCalculator::calculate_production_count(elapsed, context.tool.interval);

    state.stop();
    return count == 0 ? std::vector<mining::MiningResult>{}
                      : produce(layer, count, context);
}

bool MiningController::is_mining(const MiningState& state) const noexcept
{
    return state.is_mining();
}

bool MiningController::can_enter_layer(
    std::size_t layer_id,
    const mining::MiningContext& context
) const
{
    std::string key;
    if (!layer_key_for(layer_id, key))
        return false;

    try
    {
        if (context.mining_level < layer_table_.get_layer_level(key))
            return false;
        return check_lighting(layer_id, context);
    }
    catch (const std::out_of_range&)
    {
        return false;
    }
}

bool MiningController::check_lighting(
    std::size_t layer_id,
    const mining::MiningContext& context
) const
{
    std::string key;
    if (!layer_key_for(layer_id, key))
        return false;

    try
    {
        switch (layer_table_.get_lighting_type(key))
        {
        case Layer::LightingType::None:
            return true;
        case Layer::LightingType::Torch:
            return context.has_torch;
        case Layer::LightingType::Lantern:
            return context.has_lantern;
        }
    }
    catch (const std::out_of_range&)
    {
        return false;
    }
    return false;
}

std::vector<mining::MiningResult> MiningController::produce(
    std::size_t layer_id,
    std::size_t count,
    const mining::MiningContext& context
) const
{
    std::vector<mining::MiningResult> out;
    out.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto id = random_ore(layer_id, context);
        if (id.empty())
            continue;

        mining::MiningResult r;
        r.ore_id     = id;
        r.experience = ore_table_.get_ore_mining_exp(id);
        r.quantity   = 1;
        out.push_back(std::move(r));
    }
    return out;
}

std::string MiningController::random_ore(
    std::size_t layer_id,
    const mining::MiningContext& context
) const
{
    std::string key;
    if (!layer_key_for(layer_id, key))
        return {};

    const auto& dist = ore_table_.get_ore_distribution(key);

    double total = 0.0;
    for (const auto& [id, weight] : dist)
    {
        (void)id;
        total += weight;
    }
    if (total <= 0.0)
        return {};

    double roll = rand_unit() * total;
    for (const auto& [id, weight] : dist)
    {
        if (roll < weight)
            return id;
        roll -= weight;
    }
    return {};
}