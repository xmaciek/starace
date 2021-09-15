#pragma once

#include "widget.hpp"

#include "render_context.hpp"
#include "update_context.hpp"
#include "label.hpp"

struct HudData {
    uint32_t score = 0;
    uint32_t shots = 0;
    uint32_t pool = 0;
    uint32_t fps = 0;
    uint32_t calc = 0;
};

class Hud {
    const HudData* m_displayData = nullptr;
    Layout m_layout{ Layout::eVertical };

    Layout m_line1{ Layout::eHorizontal };
    Layout m_line2{ Layout::eHorizontal };
    Layout m_line3{ Layout::eHorizontal };
    Layout m_line4{ Layout::eHorizontal };
    Layout m_line5{ Layout::eHorizontal };

    // TODO snprintf these
    Label m_score{};
    Label m_scoreValue{};
    Label m_shots{};
    Label m_shotsValue{};
    Label m_pool{};
    Label m_poolValue{};
    Label m_fps{};
    Label m_fpsValue{};
    Label m_calc{};
    Label m_calcValue{};

public:
    Hud() noexcept = default;
    Hud( const HudData*, Font* ) noexcept;

    void render( RenderContext ) const;
    void update( const UpdateContext& );
};

