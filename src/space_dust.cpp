#include "space_dust.hpp"

#include "game_pipeline.hpp"
#include "utils.hpp"

#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <algorithm>

static math::vec4 randomPosition()
{
    return {
        randomRange( -1.0f, 1.0f ),
        randomRange( -1.0f, 1.0f ),
        randomRange( -1.0f, 1.0f ),
        0.0f,
    };
}

SpaceDust::SpaceDust() noexcept
: m_particles{ 100 }
{
    ZoneScoped;
    std::generate( m_particles.begin(), m_particles.end(), &randomPosition );
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
    assert( ( m_particles.size() % 2 ) == 0 );
    const float range = m_range;
    const math::vec4 velocity{ m_velocity * uctx.deltaTime, 0.0f };
    const math::vec4 center{ m_center, 0.0f };

    for ( auto& it : m_particles ) {
        it += velocity;
        if ( math::distance( it, center ) >= range ) {
            it = randomPosition() + center;
        }
    }
}

void SpaceDust::render( RenderContext rctx ) const
{
    ZoneScoped;
    PushConstant<Pipeline::eLine3dColor1> pushConstant{};
    pushConstant.m_model = rctx.model;
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;
    pushConstant.m_color = math::vec4{ 1.0f, 1.0f, 1.0f, 0.4f };

    PushBuffer pushBuffer{
        .m_pipeline = static_cast<PipelineSlot>( Pipeline::eLine3dColor1 ),
        .m_verticeCount = static_cast<uint32_t>( m_particles.size() * 2 ),
        .m_lineWidth = m_lineWidth,
    };
    assert( pushBuffer.m_verticeCount <= pushConstant.m_vertices.size() );

    const math::vec4 particleLength = math::vec4{ m_velocity * 0.05f, 0.0f };
    size_t i = 0;
    for ( const math::vec4& it : m_particles ) {
        pushConstant.m_vertices[ i++ ] = it;
        pushConstant.m_vertices[ i++ ] = it + particleLength;
    }
    rctx.renderer->push( pushBuffer, &pushConstant );
}
