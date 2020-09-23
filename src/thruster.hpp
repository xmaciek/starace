#pragma once

#include "circle.hpp"
#include "render_context.hpp"
#include "update_context.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <cstdint>

class Thruster {
private:
    double m_length = 1;
    double m_lengthShorter = 1;
    double m_len = 0.25;
    float m_wiggle = 0;
    glm::vec4 m_color[ 4 ]{};
    Circle m_inner;
    Circle m_outer;

public:
    Thruster( double length, double radius );

    void renderAt( RenderContext, const glm::vec3& ) const;
    void update( const UpdateContext& );
    void setColor( uint32_t num, float* colorData );
    void setLength( double newLength );
};
