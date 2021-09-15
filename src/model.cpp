#include "model.hpp"

#include "obj.hpp"
#include "texture.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

Model::~Model()
{
    destroy();
}

void Model::destroy()
{
    if ( m_vertices.m_id ) {
        Renderer::instance()->deleteBuffer( m_vertices );
    }
    if ( m_texture ) {
        destroyTexture( m_texture );
    }
}

Model::Model( const std::filesystem::path& path, Texture t, Renderer* renderer, float scale )
: m_texture{ t }
, m_scale{ scale }
{
    assert( renderer );
    loadOBJ( path.c_str(), renderer );
}

Model::Model( Model&& rhs ) noexcept
{
    std::swap( m_vertices, rhs.m_vertices );
    std::swap( m_texture, rhs.m_texture );
    std::swap( m_thrusters, rhs.m_thrusters );
    std::swap( m_weapons, rhs.m_weapons );
    std::swap( m_scale, rhs.m_scale );
}

Model& Model::operator = ( Model&& rhs ) noexcept
{
    destroy();
    m_vertices = rhs.m_vertices; rhs.m_vertices = {};
    m_texture = rhs.m_texture; rhs.m_texture = {};
    m_thrusters = std::move( rhs.m_thrusters );
    m_weapons = rhs.m_weapons; rhs.m_weapons = {};
    m_scale = rhs.m_scale; rhs.m_scale = {};
    return *this;
}

void Model::render( RenderContext rctx ) const
{
    PushConstant<Pipeline::eTriangle3dTextureNormal> pushConstant{};
    pushConstant.m_model = glm::scale( rctx.model, glm::vec3( m_scale ) );
    pushConstant.m_view = rctx.view;
    pushConstant.m_projection = rctx.projection;

    PushBuffer<Pipeline::eTriangle3dTextureNormal> pushBuffer{};
    pushBuffer.m_vertices = m_vertices;
    pushBuffer.m_texture = m_texture;

    rctx.renderer->push( &pushBuffer, &pushConstant );
}

void Model::loadOBJ( const char* filename, Renderer* renderer )
{
    using namespace std::literals::string_view_literals;
    auto data = obj::load( filename );

    for ( auto& it : data ) {
        if ( "hull"sv == it.first.name ) {
            assert( it.first.magic == obj::Chunk::c_magic );
            assert( it.first.dataType == obj::DataType::vtn );
            assert( m_vertices == Buffer::Status::eNone );
            m_vertices = renderer->createBuffer( std::move( it.second ), Buffer::Lifetime::ePersistent );
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
    assert( m_vertices != Buffer::Status::eNone );
    assert( !m_thrusters.empty() );
    assert( !m_weapons.empty() );
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
