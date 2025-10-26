#pragma once

#include "update_context.hpp"
#include "render_context.hpp"

#include <engine/math.hpp>
#include <renderer/pipeline.hpp>

#include <vector>
#include <memory_resource>

class SpaceDust
{
    std::pmr::vector<math::vec4> m_particles{};

    math::vec3 m_center{};
    math::vec3 m_velocity{};
    PipelineSlot m_pipeline{};
    float m_lineWidth = 2.0f;
    float m_range = 1.5f;

public:
    ~SpaceDust() noexcept = default;
    SpaceDust() noexcept;

    void setRange( float );
    void setLineWidth( float );
    void setVelocity( const math::vec3& );
    void setCenter( const math::vec3& );
    inline void setPipeline( PipelineSlot p ) { m_pipeline = p; }

    void update( const UpdateContext& );
    void render( const RenderContext& ) const;

};
