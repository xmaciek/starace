#include "obj.hpp"

#include <cassert>
#include <fstream>

namespace obj {

std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> load( const std::filesystem::path& path )
{

    std::ifstream ifs( path, std::ios::binary | std::ios::ate );
    assert( ifs.is_open() );

    [[maybe_unused]]
    const size_t size = ifs.tellg();
    ifs.seekg( 0 );

    assert( size >= sizeof( Header ) );

    Header header{};
    ifs.read( reinterpret_cast<char*>( &header ), sizeof( header ) );

    assert( header.magic == Header::c_magic );
    assert( header.version == Header::c_currentVersion );

    std::pmr::vector<std::pair<Chunk, std::pmr::vector<float>>> ret( header.chunkCount );

    for ( auto& it : ret ) {
        [[maybe_unused]]
        const size_t pos = ifs.tellg();
        assert( size >= pos + sizeof( Chunk ) );
        ifs.read( reinterpret_cast<char*>( &it.first ), sizeof( Chunk ) );

        assert( it.first.magic == Chunk::c_magic );
        [[maybe_unused]]
        const size_t dataPos = ifs.tellg();
        const size_t bytesToLoad = it.first.floatCount * sizeof( float );
        assert( size >= dataPos + bytesToLoad );
        it.second.resize( it.first.floatCount );
        ifs.read( reinterpret_cast<char*>( it.second.data() ), (std::streamsize)bytesToLoad );
    }

    return ret;
}

} // namespace obj
