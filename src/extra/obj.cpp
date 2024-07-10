#include <extra/obj.hpp>

#include <Tracy.hpp>

#include <cassert>
#include <cstring>
#include <fstream>

namespace obj {

std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> load( const std::filesystem::path& path )
{
    ZoneScoped;
    std::ifstream ifs( path, std::ios::binary | std::ios::ate );
    assert( ifs.is_open() );

    [[maybe_unused]]
    const std::streamsize size = ifs.tellg();
    ifs.seekg( 0 );
    std::pmr::vector<uint8_t> data( static_cast<std::size_t>( size ) );
    ifs.read( reinterpret_cast<char*>( data.data() ), size );
    ifs.close();
    return parse( std::move( data ) );
}

std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> parse( std::pmr::vector<uint8_t>&& data )
{
    ZoneScoped;
    assert( data.size() >= sizeof( Header ) );
    Header header{};
    uint8_t* ptr = data.data();
    std::memcpy( &header, ptr, sizeof( Header ) );
    std::advance( ptr, sizeof( Header ) );

    assert( header.magic == Header::MAGIC );
    assert( header.version == Header::VERSION );

    std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> ret( header.chunkCount );

    [[maybe_unused]]
    const uint8_t* end = data.data() + data.size();
    for ( auto& it : ret ) {
        assert( ptr + sizeof( Chunk ) <= end );
        std::memcpy( &it.first, ptr, sizeof( Chunk ) );
        std::advance( ptr, sizeof( Chunk ) );

        assert( it.first.magic == Chunk::MAGIC );
        const size_t bytesToLoad = it.first.floatCount * sizeof( float );
        assert( ptr + bytesToLoad <= end );
        it.second.resize( it.first.floatCount );
        std::memcpy( it.second.data(), ptr, bytesToLoad );
        std::advance( ptr, bytesToLoad );
    }

    return ret;
}

} // namespace obj
