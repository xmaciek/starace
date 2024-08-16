#include "model.hpp"

#include "colors.hpp"
#include "game_pipeline.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <cassert>
#include <algorithm>
#include <string_view>
using std::operator ""sv;

Model::Model( const Mesh& mesh, Texture t ) noexcept
: m_texture{ t }
, m_hull{ mesh[ "hull"sv ] }
{
    m_weapons[ 0 ] = mesh.m_hardpointsPrimary[ 0 ];
    m_weapons[ 1 ] = mesh.m_hardpointsSecondary[ 0 ];
    m_weapons[ 2 ] = mesh.m_hardpointsPrimary[ 1 ];
    m_thrusters.resize( mesh.m_thrusterCount );
    std::copy_n( mesh.m_thrusters.begin(), mesh.m_thrusterCount, m_thrusters.begin() );
}

void Model::render( const RenderContext& rctx ) const
{
    // assert( m_vertices );
    ZoneScoped;
    PushConstant<Pipeline::eMesh> pushConstant{
        .m_model = math::scale( rctx.model, math::vec3{ meter, meter, meter } ),
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };

    PushData pushData{
        .m_pipeline = g_pipelines[ Pipeline::eMesh ],
    };
    pushData.m_fragmentTexture[ 1 ] = m_texture;

    if ( m_hull ) {
        pushData.m_vertexBuffer = m_hull;
        rctx.renderer->push( pushData, &pushConstant );
    }
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
        pushData.m_pipeline = g_pipelines[ Pipeline::eThruster2 ];
        pushData.m_vertexBuffer = m_thruster;
        rctx.renderer->push( pushData, &p );
    }
}

std::vector<math::vec3> Model::thrusters() const
{
    ZoneScoped;
    std::vector<math::vec3> vec = m_thrusters;
    for ( auto& it : vec ) {
        it *= (float)meter;
    }
    return vec;
}

math::vec3 Model::weapon( uint32_t i ) const
{
    return m_weapons[ i >= 3 ? 0 : i ] * (float)meter;
}
