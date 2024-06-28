#include "model.hpp"

#include "game_pipeline.hpp"
#include "units.hpp"

#include <renderer/renderer.hpp>

#include <Tracy.hpp>

#include <cassert>
#include <algorithm>
#include <string_view>
using std::operator ""sv;

Model::Model( const Mesh& mesh, Texture t, float scale ) noexcept
: m_vertices{ mesh[ "hull"sv ] }
, m_texture{ t }
, m_scale{ scale }
{
    assert( m_vertices );
    m_weapons = mesh.m_hardpoints;
    m_thrusters.resize( mesh.m_thrusterCount );
    std::copy_n( mesh.m_thrusters.begin(), mesh.m_thrusterCount, m_thrusters.begin() );
}

void Model::render( RenderContext rctx ) const
{
    assert( m_vertices );
    ZoneScoped;
    PushConstant<Pipeline::eAlbedo> pushConstant{};
    const float s = m_scale;
    pushConstant.m_model = math::scale( rctx.model, math::vec3{ s, s, s } * (float)meter );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer pushBuffer{
        .m_pipeline = g_pipelines[ Pipeline::eAlbedo ],
        .m_vertexBuffer = m_vertices,
    };
    pushBuffer.m_fragmentTexture[ 1 ] = m_texture;

    rctx.renderer->push( pushBuffer, &pushConstant );
}

void Model::scale( float scale )
{
    m_scale = scale;
}

std::vector<math::vec3> Model::thrusters() const
{
    ZoneScoped;
    std::vector<math::vec3> vec = m_thrusters;
    for ( auto& it : vec ) {
        it *= m_scale * (float)meter;
    }
    return vec;
}

math::vec3 Model::weapon( uint32_t i ) const
{
    return m_weapons[ i >= 3 ? 0 : i ] * m_scale * (float)meter;
}
