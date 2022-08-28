#include "mesh.hpp"

#include <renderer/renderer.hpp>
#include <extra/obj.hpp>

#include <Tracy.hpp>

#include <cstring>

Mesh::Mesh( std::pmr::vector<uint8_t>&& data, Renderer* renderer ) noexcept
: m_renderer{ renderer }
{
    using std::literals::string_view_literals::operator""sv;
    ZoneScoped;
    uint8_t* ptr = data.data();
    [[maybe_unused]]
    const uint8_t* end = ptr + data.size();

    obj::Header header{};
    assert( data.size() >= sizeof( header ) );
    std::memcpy( &header, ptr, sizeof( header ) );
    std::advance( ptr, sizeof( header ) );

    assert( header.magic == obj::Header::c_magic );
    assert( header.version == obj::Header::c_currentVersion );

    for ( uint32_t i = 0; i < header.chunkCount; ++i ) {
        obj::Chunk chunk{};
        assert( ptr + sizeof( chunk ) <= end );
        std::memcpy( &chunk, ptr, sizeof( chunk ) );
        std::advance( ptr, sizeof( chunk ) );

        assert( chunk.magic == obj::Chunk::c_magic );
        static_assert( sizeof( math::vec3 ) == sizeof( float ) * 3 );
        // TODO: configure outside mesh file
        if ( chunk.name == "weapons"sv || chunk.name == "hardpoints"sv ) {
            assert( chunk.floatCount == 9 );
            std::memcpy( m_hardpoints.data(), ptr, sizeof( float ) * 9 );
            std::advance( ptr, sizeof( float ) * 9 );
            continue;
        }
        if ( chunk.name == "thruster"sv ) {
            assert( chunk.floatCount == 3 || chunk.floatCount == 6 );
            m_thrusterCount = chunk.floatCount / 3;
            std::memcpy( m_thrusters.data(), ptr, sizeof( float ) * chunk.floatCount );
            std::advance( ptr, sizeof( float ) * chunk.floatCount );
            continue;
        }

        const float* floats = reinterpret_cast<const float*>( ptr );
        std::span<const float> span{ floats, floats + chunk.floatCount };
        const uint32_t bytesToLoad = chunk.floatCount * sizeof( float );
        std::advance( ptr, bytesToLoad );
        Buffer buffer = renderer->createBuffer( span );
        m_map.emplace( std::make_pair( std::pmr::string{ chunk.name }, buffer ) );
    }
}

Buffer Mesh::operator [] ( std::string_view v ) const noexcept
{
    auto it = m_map.find( std::pmr::string{ v.begin(), v.end() } );
    assert( it != m_map.end() );
    return it != m_map.end() ? it->second : Buffer{};
}

Mesh::~Mesh() noexcept
{
    assert( m_renderer );
    for ( const auto& it : m_map ) {
        m_renderer->deleteBuffer( it.second );
    }
}
