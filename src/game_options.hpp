#pragma once

#include "ui_options_models.hpp"

struct OptionsGFX {
    ui::GenericDataModel m_vsync{};
    ui::GenericDataModel m_resolution{};
    ui::GenericDataModel m_fullscreen{};
    ui::OptionsArrayModel<float> m_gamma{};
};


struct OptionsCustomize {
    ui::GenericDataModel m_jet{};
    ui::GenericDataModel m_weaponPrimary{};
    ui::GenericDataModel m_weaponSecondary{};
};
