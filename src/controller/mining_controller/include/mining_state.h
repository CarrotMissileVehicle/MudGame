#pragma once

#include "mining_types.h"
#include "time_service.h"

#include <cstddef>
#include <optional>

using namespace mud;

class MiningState
{
private:
    mining::MiningStatus status_ = mining::MiningStatus::Idle;

    std::optional<std::size_t> layer_id_;

    time::gameTimePoint start_time_{};
    time::gameTimePoint last_tick_{};

public:
    bool is_mining() const noexcept;

    mining::MiningStatus status() const noexcept;

    const std::optional<std::size_t>& layer_id() const noexcept;

    time::gameTimePoint start_time() const noexcept;

    time::gameTimePoint last_tick() const noexcept;

    void start(
        std::size_t layer_id,
        mud::time::gameTimePoint now
    );

    void update_tick(
        time::gameTimePoint time
    );

    void stop() noexcept;
};