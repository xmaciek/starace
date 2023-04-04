#include "explosion.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>

#include <algorithm>

bool Explosion::isInvalid( const Explosion& e ) noexcept
{
    return e.m_state > 1.0f;
}

void Explosion::update( const UpdateContext& uctx )
{
    m_state += uctx.deltaTime;
    m_position += m_velocity * uctx.deltaTime;
}

void Explosion::renderAll( const RenderContext& rctx, std::span<const Explosion> explosions, Texture texture )
{
    if ( explosions.empty() ) return;

    using ParticleBlob = PushConstant<Pipeline::eParticleBlob>;
    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eParticleBlob ],
        .m_verticeCount = 6,
    };
    pushBuffer.m_resource[ 1 ].texture = texture;

    ParticleBlob pushConstant{
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_cameraPosition = rctx.cameraPosition,
        .m_cameraUp = rctx.cameraUp,
    };

    auto makeParticle = []( const Explosion& expl ) -> ParticleBlob::Particle
    {
        static const math::vec4 COLOR_OUT = color::crimson * math::vec4{ 1.0f, 1.0f, 1.0f, 0.0f };
        const auto& pos = expl.m_position;
        return {
            .m_position = math::vec4{ pos.x, pos.y, pos.z, math::lerp( 0.0f, expl.m_size, expl.m_state ) },
            .m_uvxywh = math::makeUVxywh<1, 1>( 0, 0 ),
            .m_color = math::lerp( expl.m_color, COLOR_OUT, expl.m_state ),
        };
    };

    auto it = explosions.begin();
    uint32_t count = std::min( static_cast<uint32_t>( std::distance( it, explosions.end() ) ), ParticleBlob::INSTANCES );
    while ( count > 0 ) {
        auto end = it;
        std::advance( end, count );
        std::transform( it, end, pushConstant.m_particles.begin(), makeParticle );
        pushBuffer.m_instanceCount = count;
        rctx.renderer->push( pushBuffer, &pushConstant );

        it = end;
        count = std::min( static_cast<uint32_t>( std::distance( it, explosions.end() ) ), ParticleBlob::INSTANCES );
    }

}