#pragma once

#include <input/action.hpp>

enum class GameAction : input::Action::Enum {
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
