#include "model.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>

#include <profiler.hpp>
#include <cassert>
#include <algorithm>
#include <string_view>
using std::operator ""sv;

Model::Model( const Mesh& mesh, Texture t ) noexcept
: m_texture{ t }
, m_hardpoints{ mesh.hardpoints }
, m_thrusterAfterglow{ mesh.thrusterAfterglow }
, m_hull{ mesh[ "hull"sv ] }
, m_thruster{ mesh[ "thruster"sv ] }
, m_wings{ mesh[ "wings"sv ] }
, m_tail{ mesh[ "tail"sv ] }
, m_intake{ mesh[ "intake"sv ] }
{
}

void Model::render( const RenderContext& rctx ) const
{
    ZoneScoped;
    PushConstant<Pipeline::eMesh> pushConstant{
        .m_model = math::scale( rctx.model, math::vec3{ meter, meter, meter } ),
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    RenderInfo ri{
        .m_pipeline = g_pipelines[ Pipeline::eMesh ],
        .m_uniform = pushConstant,
    };
    ri.m_fragmentTexture[ 0 ] = m_texture;
    auto renderMesh = [&ri, &rctx]( Buffer b )
    {
        if ( !b ) return;
        ri.m_vertexBuffer = b;
        rctx.renderer->render( ri );
    };
    renderMesh( m_tail );
    renderMesh( m_hull );
    renderMesh( m_wings );
    renderMesh( m_intake );

    if ( m_thruster ) {
        PushConstant<Pipeline::eThruster2> p{
            .m_model = math::scale( rctx.model, math::vec3{ meter, meter, meter } ),
            .m_view = rctx.view,
            .m_projection = rctx.projection,
            .m_colorInner1 = colorscheme::ion[ 1 ],
            .m_colorInner2 = colorscheme::ion[ 0 ],
            .m_colorOutter1 = colorscheme::ion[ 3 ],
            .m_colorOutter2 = colorscheme::ion[ 2 ],
        };
        ri.m_pipeline = g_pipelines[ Pipeline::eThruster2 ];
        ri.m_vertexBuffer = m_thruster;
        ri.m_uniform = p;
        rctx.renderer->render( ri );
    }
    if ( m_thrusterAfterglow.empty() ) return;

    ri.m_pipeline = g_pipelines[ Pipeline::eAfterglow ];
    ri.m_verticeCount = PushConstant<Pipeline::eAfterglow>::VERTICES;
    ri.m_instanceCount = PushConstant<Pipeline::eAfterglow>::INSTANCES;
    ri.m_vertexBuffer = {};

    static const std::array zSizeCutoff{
        math::vec4{ 0.01_m, 3.0_m, 0.1f, 0.0f },
        math::vec4{ 0.76_m, 3.0_m, 0.175f, 0.0f },
        math::vec4{ 1.53_m, 3.0_m, 0.25f, 0.0f },
        math::vec4{ 2.29_m, 3.0_m, 0.325f, 0.0f },
    };
    PushConstant<Pipeline::eAfterglow> aci{
        .m_model = rctx.model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
        .m_zSizeCutoff = zSizeCutoff,
        .m_colorScheme = colorscheme::ion,
    };
    ri.m_uniform = aci;

    for ( auto&& it : m_thrusterAfterglow ) {
        aci.m_modelOffset = it * (float)meter;
        rctx.renderer->render( ri );
    }

}
