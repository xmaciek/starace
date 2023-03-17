#pragma once

#include <ui/label.hpp>
#include "hudbar.hpp"
#include "speed_meter.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

struct HudData {
    uint32_t score = 0;
    uint32_t shots = 0;
    uint32_t pool = 0;
    uint32_t fps = 0;
    uint32_t calc = 0;
    float speed = 0;
    float hp = 0.0f;
};

class Hud {
    const HudData* m_displayData = nullptr;
    HudData m_lastData{};

    ui::Label m_score{};
    ui::Label m_scoreValue{};
    ui::Label m_shots{};
    ui::Label m_shotsValue{};
    ui::Label m_pool{};
    ui::Label m_poolValue{};
    ui::Label m_fps{};
    ui::Label m_fpsValue{};
    ui::Label m_calc{};
    ui::Label m_calcValue{};
    SpeedMeter m_speedMeter{};
    HudBar m_hp{};

public:
    Hud() noexcept = default;
    Hud( const HudData* ) noexcept;

    void render( ui::RenderContext ) const;
    void update( const UpdateContext& );

    void resize( math::vec2 );
};

