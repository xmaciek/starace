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

    for ( uint32_t i = 0; i < header.chunkCount; ++i ) {
        obj::Chunk chunk{};
        assert( ptr + sizeof( chunk ) <= end );
        std::memcpy( &chunk, ptr, sizeof( chunk ) );
        std::advance( ptr, sizeof( chunk ) );

        assert( chunk.magic == obj::Chunk::MAGIC );
        // TODO: configure outside mesh file
        if ( chunk.name == "hardpoints.primary"sv ) {
            assert( chunk.floatCount == 6 );
            for ( auto&& it : m_hardpointsPrimary ) {
                std::memcpy( &it, ptr, sizeof( float ) * 3 );
                std::advance( ptr, sizeof( float ) * 3 );
            }
            continue;
        }
        if ( chunk.name == "hardpoints.secondary"sv ) {
            assert( chunk.floatCount == 3 );
            for ( auto&& it : m_hardpointsSecondary ) {
                std::memcpy( &it, ptr, sizeof( float ) * 3 );
                std::advance( ptr, sizeof( float ) * 3 );
            }
            continue;
        }
        if ( chunk.name == "thruster.afterglow"sv ) {
            m_thrusterAfterglowCount = 0;
            switch ( chunk.floatCount ) {
            case 6:
                std::memcpy( &m_thrusterAfterglow[ m_thrusterAfterglowCount++ ], ptr, sizeof( float ) * 3 );
                std::advance( ptr, sizeof( float ) * 3 );
                [[fallthrough]];
            case 3:
                std::memcpy( &m_thrusterAfterglow[ m_thrusterAfterglowCount++ ], ptr, sizeof( float ) * 3 );
                std::advance( ptr, sizeof( float ) * 3 );
                [[fallthrough]];
            case 0:
                continue;
            default:
                assert( !"unexpected float count in thruster.afterglow" );
                break;
            }
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
    // assert( it != m_map.end() );
    return it != m_map.end() ? it->second : Buffer{};
}

Mesh::~Mesh() noexcept
{
    if ( !m_renderer ) return;
    for ( const auto& it : m_map ) {
        m_renderer->deleteBuffer( it.second );
    }
}
