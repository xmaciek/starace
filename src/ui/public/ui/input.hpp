#pragma once

#include <cstdint>
#include <engine/action.hpp>

namespace ui {

struct Action {
    enum class Enum : ::Action::Enum {
        base = 0xE000,
        eMenuUp,
        eMenuDown,
        eMenuLeft,
        eMenuRight,
        eMenuApply,
        eMenuConfirm,
        eMenuCancel,
        end,
    };
    using enum Enum;

    Enum a{};
    int16_t value = 0;

};

}

