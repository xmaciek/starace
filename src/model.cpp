#include "model.hpp"

#include "obj.hpp"
#include "texture.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

Model::~Model()
{
    destroyTexture( m_textureID );
    Renderer::instance()->deleteBuffer( m_vertices );
}

Model::Model( const char* path, Texture t, Renderer* renderer, float scale )
: m_textureID{ t }
, m_scale{ scale }
{
    loadOBJ( path );
    std::pmr::vector<float> vertices{ renderer->allocator() };
    vertices.resize( m_model.size() );
    std::copy( m_model.begin(), m_model.end(), vertices.begin() );
    assert( !vertices.empty() );
    m_vertices = renderer->createBuffer( std::move( vertices ), Buffer::Lifetime::ePersistent );
    assert( m_vertices != Buffer::Status::eNone );
}


void Model::render( RenderContext rctx ) const
{
    if ( m_vertices == Buffer::Status::eNone ) {
        std::pmr::vector<float> vertices{ rctx.renderer->allocator() };
        vertices.resize( m_model.size() );
        std::copy( m_model.begin(), m_model.end(), vertices.begin() );
        assert( !vertices.empty() );
        m_vertices = rctx.renderer->createBuffer( std::move( vertices ), Buffer::Lifetime::ePersistent );
        assert( m_vertices != Buffer::Status::eNone );
    }
    PushConstant<Pipeline::eTriangle3dTextureNormal> pushConstant{};
    pushConstant.m_model = glm::scale( rctx.model, glm::vec3( m_scale ) );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer<Pipeline::eTriangle3dTextureNormal> pushBuffer{};
    pushBuffer.m_vertices = m_vertices;
    pushBuffer.m_texture = m_textureID;

    rctx.renderer->push( &pushBuffer, &pushConstant );
}

void Model::loadOBJ( const char* filename )
{
    using namespace std::literals::string_view_literals;
    auto data = obj::load( filename );

    for ( auto& it : data ) {
        if ( "hull"sv == it.first.name ) {
            assert( it.first.magic == obj::Chunk::c_magic );
            assert( it.first.dataType == obj::DataType::vtn );
            m_model = std::move( it.second );
        }
        else if ( "weapons"sv == it.first.name ) {
            assert( it.first.magic == obj::Chunk::c_magic );
            assert( it.first.dataType == obj::DataType::v );
            float* ptr = reinterpret_cast<float*>( m_weapons.data() );
            const size_t toCopy = std::min( it.second.size(), m_weapons.size() * 3 );
            std::copy_n( it.second.data(), toCopy, ptr );
        }
        else if ( "thruster"sv == it.first.name ) {
            assert( it.first.magic == obj::Chunk::c_magic );
            assert( it.first.dataType == obj::DataType::v );
            m_thrusters.resize( it.second.size() / 3 );
            float* ptr = reinterpret_cast<float*>( m_thrusters.data() );
            std::copy_n( it.second.data(), it.second.size(), ptr );
        }
    }
    assert( !m_model.empty() );
    assert( !m_thrusters.empty() );
    assert( !m_weapons.empty() );
}

void Model::bindTexture( Texture tex )
{
    if ( m_textureID ) {
        destroyTexture( m_textureID );
    }
    m_textureID = tex;
}

void Model::scale( float scale )
{
    m_scale = scale;
}

std::vector<glm::vec3> Model::thrusters() const
{
    std::vector<glm::vec3> vec = m_thrusters;
    for ( auto& it : vec ) {
        it *= m_scale;
    }
    return vec;
}

glm::vec3 Model::weapon( uint32_t i ) const
{
    return m_weapons[ i >= 3 ? 0 : i ] * m_scale;
}
