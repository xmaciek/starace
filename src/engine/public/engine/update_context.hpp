#pragma once

#include <engine/math.hpp>

struct UpdateContext {
    math::mat4 camera{ 1.0f };
    math::vec2 viewport{};
    float deltaTime = 0.0f;
};
