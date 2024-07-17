#include "model.hpp"

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
    // assert( m_vertices );
    m_weapons = mesh.m_hardpoints;
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

    auto testRender = [&pushData, &pushConstant, rend=rctx.renderer]( auto buff )
    {
        if ( !buff ) return;
        pushData.m_vertexBuffer = buff;
        rend->push( pushData, &pushConstant );
    };
    testRender( m_elevators );
    testRender( m_fins );
    testRender( m_engines );
    testRender( m_wings );
    testRender( m_hull );
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
