#pragma once

#include "ui_options_models.hpp"


struct OptionsGFX {
    ui::OptionsArrayModel<float> m_gamma{};

    ui::Option<bool> m_fullscreen{ 0 };
    ui::Option<VSync> m_vsync{ 1
        , { VSync::eOff, VSync::eOn, VSync::eMailbox }
        , { "off"_hash, "on"_hash, "mailbox"_hash }
    };
    ui::Option<DisplayMode> m_resolution{};

};


struct OptionsCustomize {
    ui::GenericDataModel m_jet{};
    ui::GenericDataModel m_weaponPrimary{};
    ui::GenericDataModel m_weaponSecondary{};
};
