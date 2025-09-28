#pragma once

#include <engine/math.hpp>

#include <cstdint>

struct MouseEvent {
    enum class Type : uint16_t {
        eMove,
        eClick,
        eClickSecondary,
        eClickMiddle,
    };
    using enum Type;

    Type type{};
    int16_t value{};
    math::vec2 position{};
};
