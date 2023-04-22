#pragma once

#include <engine/action.hpp>

enum class GameAction : Action::Enum {
    eGamePause,
    eJetPitch,
    eJetYaw,
    eJetRoll,
    eJetShoot1,
    eJetShoot2,
    eJetShoot3,
    eJetTarget,
    eJetSpeed,
    eJetLookAt,
};
