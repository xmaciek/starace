#pragma once

#include <engine/math.hpp>

struct MouseEvent {
    enum class Type : uint32_t {
        eMove,
        eClick,
    };
    using enum Type;

    Type type{};
    math::vec2 position{};
};
