#pragma once

#include "ui_options_models.hpp"


struct OptionsGFX {
    ui::GenericDataModel m_vsync{};
    ui::GenericDataModel m_resolution{};
    ui::OptionsArrayModel<float> m_gamma{};

    ui::Option<bool> m_fullscreen{ 0 };
    ui::Option<VSync> testOption{ 2
        , { "off"_hash, "on"_hash, "mailbox"_hash }
        , { VSync::eOff, VSync::eOn, VSync::eMailbox }
    };

};


struct OptionsCustomize {
    ui::GenericDataModel m_jet{};
    ui::GenericDataModel m_weaponPrimary{};
    ui::GenericDataModel m_weaponSecondary{};
};
