#pragma once

#include <engine/math.hpp>
#include <engine/render_context.hpp>
#include <engine/update_context.hpp>

class Shield {
private:
    math::vec4 m_color{};
    float m_rotAngle = 0.0;
    float m_radius = 0.0;

public:
    Shield() = default;
    Shield( double radiusA, double radiusB );

    double radius();
    void render( RenderContext ) const;
    void setColor( const math::vec4& );
    void update( const UpdateContext& );
};
