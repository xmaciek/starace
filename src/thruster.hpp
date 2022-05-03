#pragma once

#include "circle.hpp"
#include "colors.hpp"
#include "units.hpp"

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/buffer.hpp>

#include <cstdint>

class Thruster {
private:
    math::vec2 m_lengthRange{};
    float m_length = 10.0_m;
    float m_wiggle = 0.0f;
    ColorScheme m_colorScheme = colorscheme::ion;

public:
    Thruster() = default;
    Thruster( float length, float radius );

    void renderAt( RenderContext, const math::vec3& ) const;
    void update( const UpdateContext& );
    void setColorScheme( const ColorScheme& );
    void setLength( float );
};
