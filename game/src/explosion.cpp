#include "explosion.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"

#include <renderer/renderer.hpp>
#include <profiler.hpp>

#include <algorithm>
#include <ranges>

void Explosion::renderAll( const RenderContext& rctx, const std::pmr::vector<Explosion>& explosions )
{
    ZoneScoped;
    if ( explosions.empty() ) return;

    using Uniform = PushConstant<Pipeline::eParticleBlob>;
    Uniform uniform{
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_cameraPosition = rctx.cameraPosition,
        .m_cameraUp = rctx.cameraUp,
    };
    RenderInfo ri{
        .m_pipeline = g_pipelines[ Pipeline::eParticleBlob ],
        .m_verticeCount = uniform.VERTICES,
    };
    ri.m_uniforms.emplace_back( uniform );
    ri.m_fragmentTexture.emplace_back( explosions.front().m_texture ); // TODO sort + split by texture

    auto makeParticle = []( const Explosion& expl ) -> Uniform::Instance
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

    auto submitParticles = [&rctx, &ri, &uniform, makeParticle]( auto&& batch )
    {
        std::ranges::transform( batch, uniform.m_instances.begin(), makeParticle );
        ri.m_instanceCount = static_cast<uint32_t>( batch.size() );
        rctx.renderer->render( ri );
    };
    auto batches = explosions | std::views::chunk( uniform.INSTANCES );
    std::ranges::for_each( batches, submitParticles );
}

void Explosion::updateAll( const UpdateContext& uctx, std::pmr::vector<Explosion>& vec )
{
    ZoneScoped;
    std::ranges::for_each( vec, [dt=uctx.deltaTime]( auto& e ) { e.m_state += dt; e.m_position += e.m_velocity * dt; } );
    std::erase_if( vec, []( const auto& e ) { return e.m_state >= e.m_duration; } );
}
