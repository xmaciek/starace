#pragma once

#include <engine/math.hpp>

#include <variant>

struct MouseEvent {
    enum Type : uint32_t {
        eMove,
        eClick,
    };
    Type type{};
    math::vec2 position{};
};
