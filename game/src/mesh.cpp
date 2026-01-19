#include "mesh.hpp"

#include <renderer/renderer.hpp>
#include <extra/obj.hpp>

#include <profiler.hpp>

#include <cstring>

Mesh::Mesh( std::span<const uint8_t> data, Renderer* renderer ) noexcept
: m_renderer{ renderer }
{
    using std::literals::string_view_literals::operator""sv;
    ZoneScoped;
    const uint8_t* ptr = data.data();
    [[maybe_unused]]
    const uint8_t* end = ptr + data.size();

    obj::Header header{};
    assert( data.size() >= sizeof( header ) );
    std::memcpy( &header, ptr, sizeof( header ) );
    std::advance( ptr, sizeof( header ) );

    assert( header.magic == obj::Header::MAGIC );
    assert( header.version == obj::Header::VERSION );

    auto readVec3 = []( auto& vec, auto& ptr )
    {
        for ( auto&& it : vec ) {
            std::memcpy( &it, ptr, sizeof( float ) * 3 );
            std::advance( ptr, sizeof( float ) * 3 );
        }
    };
    for ( uint32_t i = 0; i < header.chunkCount; ++i ) {
        obj::Chunk chunk{};
        assert( ptr + sizeof( chunk ) <= end );
        std::memcpy( &chunk, ptr, sizeof( chunk ) );
        std::advance( ptr, sizeof( chunk ) );

        assert( chunk.magic == obj::Chunk::MAGIC );
        // TODO: configure outside mesh file
        if ( chunk.name == "hardpoints.primary"sv ) {
            assert( chunk.floatCount == 6 );
            hardpoints.primary.resize( 2 );
            readVec3( hardpoints.primary, ptr );
            continue;
        }
        if ( chunk.name == "hardpoints.secondary"sv ) {
            assert( chunk.floatCount == 3 );
            hardpoints.secondary.resize( 1 );
            readVec3( hardpoints.secondary, ptr );
            continue;
        }
        if ( chunk.name == "thruster.afterglow"sv ) {
            assert( chunk.floatCount % 3 == 0 );
            assert( chunk.floatCount <= 6 );
            thrusterAfterglow.resize( chunk.floatCount / 3 );
            readVec3( thrusterAfterglow, ptr );
            continue;
        }

        const uint8_t* floats = reinterpret_cast<const uint8_t*>( ptr );
        const uint32_t bytesToLoad = chunk.floatCount * sizeof( float );
        std::span<const uint8_t> span{ floats, bytesToLoad };
        std::advance( ptr, bytesToLoad );
        Buffer buffer = renderer->createBuffer( span );
        m_map.emplace( std::make_pair( std::pmr::string{ chunk.name }, buffer ) );
    }
}

Buffer Mesh::operator [] ( std::string_view v ) const noexcept
{
    auto it = m_map.find( std::pmr::string{ v.begin(), v.end() } );
    return it != m_map.end() ? it->second : Buffer{};
}

Mesh::~Mesh() noexcept
{
    if ( !m_renderer ) return;
    for ( const auto& it : m_map ) {
        m_renderer->deleteBuffer( it.second );
    }
}
