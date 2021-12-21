#pragma once

#include <engine/action.hpp>

enum class GameAction : Action::Enum {
    eMenuConfirm,
    eMenuCancel,
    eMenuUp,
    eMenuDown,
    eMenuLeft,
    eMenuRight,
};
