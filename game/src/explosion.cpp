#include "explosion.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>

#include <algorithm>

bool Explosion::isInvalid( const Explosion& e ) noexcept
{
    return ( e.m_state / e.m_duration ) >= 1.0f;
}

void Explosion::renderAll( const RenderContext& rctx, const std::pmr::vector<Explosion>& explosions )
{
    if ( explosions.empty() ) return;

    using ParticleBlob = PushConstant<Pipeline::eParticleBlob>;
    ParticleBlob pushConstant{
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_cameraPosition = rctx.cameraPosition,
        .m_cameraUp = rctx.cameraUp,
    };
    RenderInfo ri{
        .m_pipeline = g_pipelines[ Pipeline::eParticleBlob ],
        .m_verticeCount = PushConstant<Pipeline::eParticleBlob>::VERTICES,
        .m_uniform = pushConstant,
    };
    ri.m_fragmentTexture[ 0 ] = explosions.front().m_texture; // TODO sort + split by texture



    auto makeParticle = []( const Explosion& expl ) -> ParticleBlob::Particle
    {
        static const math::vec4 COLOR_OUT = color::crimson * math::vec4{ 1.0f, 1.0f, 1.0f, 0.0f };
        const auto& pos = expl.m_position;
        float state = expl.m_state / expl.m_duration;
        uint32_t idx = static_cast<uint32_t>( 60.0f * expl.m_state ) % 4; // anim fps
        return {
            .m_position = math::vec4{ pos.x, pos.y, pos.z, math::lerp( 0.0f, expl.m_size, state ) },
            .m_uvxywh = math::makeUVxywh<2, 2>( ( idx / 2 ) % 2, idx % 2 ),
            .m_color = math::lerp( expl.m_color, COLOR_OUT, math::smoothstep( 0.0f, 1.0f, state ) ),
        };
    };

    auto it = explosions.begin();
    uint32_t count = std::min( static_cast<uint32_t>( std::distance( it, explosions.end() ) ), ParticleBlob::INSTANCES );
    while ( count > 0 ) {
        auto end = it;
        std::advance( end, count );
        std::transform( it, end, pushConstant.m_particles.begin(), makeParticle );
        ri.m_instanceCount = count;
        rctx.renderer->render( ri );

        it = end;
        count = std::min( static_cast<uint32_t>( std::distance( it, explosions.end() ) ), ParticleBlob::INSTANCES );
    }

}

void Explosion::updateAll( const UpdateContext& uctx, std::pmr::vector<Explosion>& vec )
{
    std::for_each( vec.begin(), vec.end(), [dt=uctx.deltaTime]( auto& e ) { e.m_state += dt; e.m_position += e.m_velocity * dt; } );
    std::erase_if( vec, &isInvalid );
}
