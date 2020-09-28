#include "map.hpp"

#include "utils.hpp"
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Map::render( RenderContext rctx )
{
    static Buffer walls[ 6 ]{};
    static Buffer uv[ 6 ]{};

    // NOTE: this is so bad, should ba model rendering
    if ( !walls[ 0 ] ) {
        std::pmr::memory_resource* a = rctx.renderer->allocator();
        using Vec = std::pmr::vector<glm::vec3>;
        using Uv = std::pmr::vector<glm::vec2>;
        std::pmr::vector<glm::vec3> w[ 6 ] = { Vec{ 4, a },Vec{ 4, a },Vec{ 4, a },Vec{ 4, a },Vec{ 4, a },Vec{ 4, a } };
        std::pmr::vector<glm::vec2> auv[ 6 ] ={ Uv{ 4, a }, Uv{ 4, a }, Uv{ 4, a }, Uv{ 4, a }, Uv{ 4, a }, Uv{ 4, a } };

        int i = 0;
        int j = 0;

        w[ i ][ j++ ] = glm::vec3{ -m_v1, -m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, -m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, m_v1, -m_v1 };
        j = 0;
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_max };
        walls[ i ] = rctx.renderer->createBuffer( std::move( w[ i ] ), Buffer::Lifetime::ePersistent );
        uv[ i ] = rctx.renderer->createBuffer( std::move( auv[ i ] ), Buffer::Lifetime::ePersistent  );
        i++;
        j = 0;

        w[ i ][ j++ ] = glm::vec3{ -m_v1, m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, -m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, -m_v1, m_v1 };
        j = 0;
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_min };
        walls[ i ] = rctx.renderer->createBuffer( std::move( w[ i ] ), Buffer::Lifetime::ePersistent  );
        uv[ i ] = rctx.renderer->createBuffer( std::move( auv[ i ] ), Buffer::Lifetime::ePersistent  );
        i++;
        j = 0;

        w[ i ][ j++ ] = glm::vec3{ -m_v1, -m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, -m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, m_v1, m_v1 };
        j = 0;
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_max };
        walls[ i ] = rctx.renderer->createBuffer( std::move( w[ i ] ), Buffer::Lifetime::ePersistent  );
        uv[ i ] = rctx.renderer->createBuffer( std::move( auv[ i ] ), Buffer::Lifetime::ePersistent  );
        i++;
        j = 0;

        w[ i ][ j++ ] = glm::vec3{ m_v1, m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, -m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, -m_v1, m_v1 };
        j = 0;
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_min };
        walls[ i ] = rctx.renderer->createBuffer( std::move( w[ i ] ), Buffer::Lifetime::ePersistent  );
        uv[ i ] = rctx.renderer->createBuffer( std::move( auv[ i ] ), Buffer::Lifetime::ePersistent  );
        i++;
        j = 0;

        w[ i ][ j++ ] = glm::vec3{ -m_v1, m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, m_v1, m_v1 };
        j = 0;
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_max };
        walls[ i ] = rctx.renderer->createBuffer( std::move( w[ i ] ), Buffer::Lifetime::ePersistent  );
        uv[ i ] = rctx.renderer->createBuffer( std::move( auv[ i ] ), Buffer::Lifetime::ePersistent  );
        i++;
        j = 0;

        w[ i ][ j++ ] = glm::vec3{ -m_v1, -m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, -m_v1, m_v1 };
        w[ i ][ j++ ] = glm::vec3{ m_v1, -m_v1, -m_v1 };
        w[ i ][ j++ ] = glm::vec3{ -m_v1, -m_v1, -m_v1 };
        j = 0;
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_min };
        auv[ i ][ j++ ] = glm::vec2{ m_max, m_max };
        auv[ i ][ j++ ] = glm::vec2{ m_min, m_max };
        walls[ i ] = rctx.renderer->createBuffer( std::move( w[ i ] ), Buffer::Lifetime::ePersistent  );
        uv[ i ] = rctx.renderer->createBuffer( std::move( auv[ i ] ), Buffer::Lifetime::ePersistent  );
    }

    {
        PushConstant<Pipeline::eTriangleFan3dTexture> pushConstant{};
        pushConstant.m_model = rctx.model;
        pushConstant.m_view = rctx.view;
        pushConstant.m_projection = rctx.projection;

        const uint32_t tex[ 6 ] = { m_back, m_front, m_left, m_right, m_top, m_bottom };
        PushBuffer<Pipeline::eTriangleFan3dTexture> pushBuffer{};
        for ( int i = 0; i < 6; i++ ) {
            pushBuffer.m_texture = tex[ i ];
            pushBuffer.m_vertices = walls[ i ];
            pushBuffer.m_uv = uv[ i ];
            rctx.renderer->push( &pushBuffer, &pushConstant );
        }
    }

    {
        PushConstant<Pipeline::eLine3dColor1> linePushConstant{};
        linePushConstant.m_model = rctx.model;
        linePushConstant.m_view = rctx.view;
        linePushConstant.m_projection = rctx.projection;
        linePushConstant.m_color = glm::vec4{ 1.0f, 1.0f, 1.0f, 0.4f };
        PushBuffer<Pipeline::eLine3dColor1> linePushBuffer{};

        std::pmr::vector<glm::vec3> particles{ rctx.renderer->allocator() };
        particles.reserve( m_particleList.size() * 2 );
        for ( const glm::vec3& it : m_particleList ) {
            particles.emplace_back( it );
            particles.emplace_back( it + m_particleLength );
        }
        linePushBuffer.m_vertices = rctx.renderer->createBuffer( std::move( particles ), Buffer::Lifetime::eOneTimeUse );
        rctx.renderer->push( &linePushBuffer, &linePushConstant );
    }
}

void Map::update( const UpdateContext& updateContext )
{
    const glm::vec3 tmpVelocity = m_jetVelocity * -0.1f * updateContext.deltaTime;
    for ( auto& it : m_particleList ) {
        it += tmpVelocity;
        if ( glm::distance( it, m_jetPosition ) >= 1.5 ) {
            it.x = randomRange( m_jetPosition.x - 1, m_jetPosition.x + 1 );
            it.y = randomRange( m_jetPosition.y - 1, m_jetPosition.y + 1 );
            it.z = randomRange( m_jetPosition.z - 1, m_jetPosition.z + 1 );
        }
    }
}

Map::Map( const MapProto& data )
{
    m_top = loadTexture( data.TOP.c_str() );
    m_bottom = loadTexture( data.BOTTOM.c_str() );
    m_left = loadTexture( data.LEFT.c_str() );
    m_right = loadTexture( data.RIGHT.c_str() );
    m_front = loadTexture( data.FRONT.c_str() );
    m_back = loadTexture( data.BACK.c_str() );

    m_min = 0.00125;
    m_max = 0.99875;

    m_particleList.reserve( 100 );
    for ( int i = 0; i < 100; i++ ) {
        m_particleList.emplace_back( randomRange( -1, 1 ), randomRange( -1, 1 ), randomRange( -1, 1 ) );
    }

    m_v1 = 1000;
    m_v2 = 100;
}

void Map::setJetData( const glm::vec3& position, const glm::vec3& velocity )
{
    m_jetPosition = position;
    m_jetVelocity = velocity;
    m_particleLength = m_jetVelocity * 0.05f;
}

Map::~Map()
{
    destroyTexture( m_top );
    destroyTexture( m_bottom );
    destroyTexture( m_left );
    destroyTexture( m_right );
    destroyTexture( m_front );
    destroyTexture( m_back );
}
