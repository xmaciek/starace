#pragma once

#include <cstdint>
#include <engine/action.hpp>

namespace ui {

struct Action {
    enum class Enum : ::Action::Enum {
        base = 256,
        eMenuUp,
        eMenuDown,
        eMenuLeft,
        eMenuRight,
        eMenuConfirm,
        eMenuCancel,
        end,
    };
    using enum Enum;

    Enum a{};
    int16_t value = 0;
};

}
