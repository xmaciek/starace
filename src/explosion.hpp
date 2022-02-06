#pragma once

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/texture.hpp>

struct Explosion
{
    math::vec3 m_position{};
    math::vec3 m_velocity{};
    Texture m_texture{};
    float m_state = 0.0f;

    static bool isInvalid( const Explosion& ) noexcept;
    void update( const UpdateContext& );
    void render( const RenderContext& ) const;
};
