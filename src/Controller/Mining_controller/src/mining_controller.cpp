/**
 * @file mining_controller.cpp
 * @brief 采矿控制器实现。
 *
 * 驱动采矿会话：启动校验、进度结算与产出计算。层/矿石数据来自
 * 内部 Ore/Layer 查询表（由 JSON 数据加载）。
 */
#include "mining_controller.h"

#include "mining_calculator.h"

#include "event.h"
#include "tool.h"
#include "tool_controller.h"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
    // size_t 层 id → JSON 层 key（与 mining::MiningLayer 枚举序一致）。
    bool layer_key_for(const std::size_t layer_id, std::string& out)
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
    const TimeService& time_service,
    const mud::event::EventSystem* events,
    mud::tool::ToolController* tools
) : ore_data_(ore_data), time_service_(time_service),
    events_(events), tools_(tools)
{
}

bool MiningController::start_mining(
    MiningState& state,
    const std::size_t layer_id,
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

    bool interrupted = false;
    auto results = produce(*state.layer_id(), count, context, interrupted);
    if (interrupted)
        state.stop(); // 塌方/工具损坏：清空本 tick 产出并中断会话
    return results;
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
    bool interrupted = false;
    return count == 0 ? std::vector<mining::MiningResult>{}
                      : produce(layer, count, context, interrupted);
}

bool MiningController::is_mining(const MiningState& state) const noexcept
{
    return state.is_mining();
}

bool MiningController::can_enter(
    const std::size_t layer_id,
    const mining::MiningContext& context
) const
{
    return can_enter_layer(layer_id, context);
}

std::optional<mining::MiningResult> MiningController::produce_once(
    const std::size_t layer_id,
    const mining::MiningContext& context
)
{
    bool interrupted = false;
    auto results = produce(layer_id, 1, context, interrupted);
    if (interrupted || results.empty())
        return std::nullopt;
    return std::move(results.front());
}

bool MiningController::can_enter_layer(
    const std::size_t layer_id,
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
    const std::size_t layer_id,
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
    const std::size_t layer_id,
    const std::size_t count,
    const mining::MiningContext& context,
    bool& interrupted
)
{
    std::vector<mining::MiningResult> out;
    out.reserve(count);
    interrupted = false;

    for (std::size_t i = 0; i < count; ++i)
    {
        // 1. 事件判定：宝箱翻倍 / 塌方中断本 tick。
        bool chest = false, cave = false;
        if (events_)
            events_->roll_mining_event(chest, cave,
                static_cast<int>(rand_unit() * 100) + 1);
        if (cave)
        {
            interrupted = true;
            out.clear();
            return out;
        }

        const auto id = random_ore(layer_id, context);
        if (id.empty())
            continue;

        mining::MiningResult r;
        r.ore_id     = id;
        r.quantity   = chest ? 2 : 1;  // 2. 宝箱：当次产出翻倍

        // 3. 矿镐耐久：每次产出扣 1；损坏即中断。
        if (tools_ && !tools_->use_tool(mud::tool::ToolId::Pickaxe))
        {
            interrupted = true;
            out.clear();
            return out;
        }

        // 4. 经验：baseExp * (1 + 矿镐等级加成)。
        const int bonus = tools_ ? tools_->level_bonus(mud::tool::ToolId::Pickaxe) : 0;
        r.experience = ore_table_.get_ore_mining_exp(id) * (1 + bonus);

        out.push_back(std::move(r));
    }
    return out;
}

std::string MiningController::random_ore(
    const std::size_t layer_id,
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