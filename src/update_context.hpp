#pragma once

#include "signal.hpp"

#include <span>

struct UpdateContext {
    float deltaTime = 0.0f;
    std::span<const Signal> signals{};
};
