#pragma once

#include "circle.hpp"
#include "colors.hpp"

#include <engine/render_context.hpp>
#include <engine/update_context.hpp>
#include <renderer/buffer.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstdint>

class Thruster {
private:
    glm::vec2 m_lengthRange{};
    float m_length = 0.25f;
    float m_wiggle = 0.0f;
    ColorScheme m_colorScheme = colorscheme::ion;

public:
    Thruster() = default;
    Thruster( float length, float radius );

    void renderAt( RenderContext, const glm::vec3& ) const;
    void update( const UpdateContext& );
    void setColorScheme( const ColorScheme& );
    void setLength( float );
};
