#pragma once

#include <engine/action.hpp>

enum class GameAction : Action::Enum {
    eMenuConfirm,
    eMenuCancel,
    eMenuUp,
    eMenuDown,
    eMenuLeft,
    eMenuRight,

    eJetPitch,
    eJetYaw,
    eJetRoll,
    eJetShoot1,
    eJetShoot2,
    eJetShoot3,
    eJetTarget,
};
