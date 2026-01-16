#include "explosion.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>

#include <algorithm>

void Explosion::renderAll( const RenderContext& rctx, const std::pmr::vector<Explosion>& explosions )
{
    if ( explosions.empty() ) return;

    using Instanced = InstancedRendering<PushConstant<Pipeline::eParticleBlob>>;
    Instanced instanced{ rctx.renderer, g_pipelines[ Pipeline::eParticleBlob ] };

    instanced.pushConstant.m_view = rctx.view;
    instanced.pushConstant.m_projection = rctx.projection;
    instanced.pushConstant.m_cameraPosition = rctx.cameraPosition;
    instanced.pushConstant.m_cameraUp = rctx.cameraUp;
    instanced.renderInfo.m_fragmentTexture[ 0 ] = explosions.front().m_texture; // TODO sort + split by texture


    auto makeParticle = []( const Explosion& expl ) -> Instanced::Instance
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

    std::ranges::for_each( explosions, [&instanced, makeParticle]( const auto& expl )
        { instanced.append( makeParticle( expl ) ); }
    );
}

void Explosion::updateAll( const UpdateContext& uctx, std::pmr::vector<Explosion>& vec )
{
    std::ranges::for_each( vec, [dt=uctx.deltaTime]( auto& e ) { e.m_state += dt; e.m_position += e.m_velocity * dt; } );
    std::erase_if( vec, []( const auto& e ) { return e.m_state >= e.m_duration; } );
}
