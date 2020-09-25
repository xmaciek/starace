#pragma once

#include "circle.hpp"
#include "render_context.hpp"
#include "update_context.hpp"
#include <renderer/buffer.hpp>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstdint>

class Thruster {
private:
    float m_length = 1.0f;
    float m_lengthShorter = 1.0f;
    float m_len = 0.25f;
    float m_wiggle = 0.0f;
    glm::vec4 m_color[ 4 ]{};
    Circle m_inner;
    Circle m_outer;
    static Buffer s_innerCone[ 2 ];
    static Buffer s_outerCone[ 2 ];

public:
    Thruster( double length, double radius );

    void renderAt( RenderContext, const glm::vec3& ) const;
    void update( const UpdateContext& );
    void setColor( uint32_t num, float* colorData );
    void setLength( double newLength );
};
