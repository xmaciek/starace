#pragma once

#include <engine/math.hpp>
#include <engine/update_context.hpp>
#include <engine/render_context.hpp>

#include <vector>
#include <memory_resource>

class SpaceDust
{
    std::pmr::vector<math::vec4> m_particles{};

    math::vec3 m_center{};
    math::vec3 m_velocity{};
    float m_lineWidth = 2.0f;
    float m_range = 1.5f;

public:
    ~SpaceDust() noexcept = default;
    SpaceDust() noexcept;

    void setRange( float );
    void setLineWidth( float );
    void setVelocity( const math::vec3& );
    void setCenter( const math::vec3& );

    void update( const UpdateContext& );
    void render( RenderContext ) const;

};
