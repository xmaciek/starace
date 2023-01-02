#pragma once

#include <engine/math.hpp>

#include <variant>

struct MouseEvent {
    enum class Type : uint32_t {
        eMove,
        eClick,
    };
    using enum Type;

    enum class Processing : uint32_t {
        eStop,
        eContinue,
    };
    using enum Processing;

    Type type{};
    math::vec2 position{};
};
