#pragma once

#include "circle.hpp"
#include "sa.hpp"
#include "render_context.hpp"
#include "update_context.hpp"

#include <glm/vec4.hpp>

class Shield {
private:
    glm::vec4 m_color{};
    Circle m_circle{};
    float m_rotAngle = 0.0;
    float m_radius = 0.0;

public:
    Shield() = default;
    Shield( double radiusA, double radiusB );

    double radius();
    void render( RenderContext ) const;
    void setColor( const glm::vec4& );
    void update( const UpdateContext& );
};
