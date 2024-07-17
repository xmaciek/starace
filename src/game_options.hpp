#pragma once

#include "ui_options_models.hpp"

#include <ui/var.hpp>
#include <renderer/buffer.hpp>

#include <string>
#include <memory_resource>

struct OptionsGFX {
    ui::Option<DisplayMode> m_resolution{};
    ui::Option<bool> m_fullscreen{ 0 };
    ui::Option<VSync> m_vsync{};

    ui::Option<float> m_gamma{ 18,
        std::pmr::vector<float>{ 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f
            , 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f
            , 2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f
            , 3.0f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f
            , 4.0f, 4.1f, 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f
        }
        , &toString<float>
    };
};

using JetPart = std::pair<Buffer, std::pmr::string>;
using PartModel = ui::Option<JetPart>;
struct OptionsCustomize {
    ui::GenericDataModel m_jet{};
    ui::GenericDataModel m_weaponPrimary{};
    ui::GenericDataModel m_weaponSecondary{};
    PartModel m_hull{};
    PartModel m_engines{};
    PartModel m_wings{};
    PartModel m_elevators{};
    PartModel m_fins{};
};

struct GameplayUIData {
    ui::Var<float> m_playerHP{ 0.0f };
    ui::Var<float> m_jetSpeed{ 0.0f };
};
