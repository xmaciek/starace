#pragma once

#include "units.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>

#include <span>

struct Explosion {
    math::vec3 m_position{};
    math::vec3 m_velocity{};
    math::vec4 m_color{};
    Texture m_texture{};
    float m_size = 64.0_m;
    float m_state = 0.0f;

    static bool isInvalid( const Explosion& ) noexcept;
    void update( const UpdateContext& );
    static void renderAll( const RenderContext&, std::span<const Explosion>, Texture );
};
