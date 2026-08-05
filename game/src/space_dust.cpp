#include "space_dust.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"

#include <renderer/renderer.hpp>

#include <profiler.hpp>

#include <algorithm>

static math::vec4 randomPosition()
{
    return {
        randomRange( -1.0f, 1.0f ),
        randomRange( -1.0f, 1.0f ),
        randomRange( -1.0f, 1.0f ),
        1.0f,
    };
}

SpaceDust::SpaceDust() noexcept
{
    ZoneScoped;
    m_particles.resize( 100 );
    std::ranges::generate( m_particles, &randomPosition );
}

void SpaceDust::setVelocity( const math::vec3& v )
{
    m_velocity = v;
}

void SpaceDust::setCenter( const math::vec3& v )
{
    m_center = v;
}

void SpaceDust::setRange( float r )
{
    m_range = r;
}

void SpaceDust::setLineWidth( float w )
{
    m_lineWidth = w;
}

void SpaceDust::update( const UpdateContext& uctx )
{
    ZoneScoped;
    const math::vec4 velocity{ m_velocity * uctx.deltaTime, 0.0f };
    const math::vec4 center{ m_center, 0.0f };

    std::ranges::for_each( m_particles, [velocity, center, r=m_range]( auto& it )
    {
        it += velocity;
        if ( math::distance( it, center ) >= r ) {
            it = randomPosition() + center;
        }
    });
}

void SpaceDust::render( const RenderContext& rctx ) const
{
    ZoneScoped;

    PushConstant<Pipeline::eSpaceDust> pushConstant{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 0.4f },
        .m_particleOffset = math::vec4{ m_velocity * 0.05f, 0.0f },
    };
    RenderInfo ri{
        .m_pipeline = m_pipeline,
        .m_verticeCount = pushConstant.VERTICES,
        .m_instanceCount = static_cast<uint32_t>( m_particles.size() ),
        .m_lineWidth = m_lineWidth * ( rctx.viewport.y / 720.0f ),
    };
    ri.m_uniforms.emplace_back( pushConstant );
    ri.m_uniforms.emplace_back( m_particles );
    rctx.renderer->render( ri );
}
