#include <engine/async_io.hpp>
#include <extra/tar.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <utility>

AsyncIO::~AsyncIO() noexcept
{
    m_isRunning.store( false );

}

AsyncIO::AsyncIO() noexcept
{
}


std::span<const uint8_t> AsyncIO::viewWait( const std::filesystem::path& path )
{
    ZoneScopedN( "AsyncIO viewWait" );
    while ( m_isRunning.load() ) {
        std::scoped_lock<std::mutex> sl( m_bottleneck );
        auto it = m_blobView.find( path );
        if ( it != m_blobView.end() ) {
            return it->second;
        }
        assert( !"TOO SOON" );
    }
    return {};
}

void AsyncIO::mount( const std::filesystem::path& path )
{
    ZoneScopedN( "TAR load" );
    if ( path.extension() != ".tar" ) {
        assert( !"archive not .tar" );
        return;
    }
    std::ifstream ifs( path, std::ios::binary | std::ios::ate );
    assert( ifs.is_open() );

    std::pmr::vector<uint8_t> blob( static_cast<std::pmr::vector<uint8_t>::size_type>( ifs.tellg() ) );
    ifs.seekg( 0 );
    ifs.read( reinterpret_cast<char*>( blob.data() ), static_cast<std::streamsize>( blob.size() ) );
    ifs.close();

    const tar::Header* header = reinterpret_cast<tar::Header*>( blob.data() );
    for ( ; tar::isHeaderValid( header ); header = tar::next( header ) ) {
        std::scoped_lock<std::mutex> sl( m_bottleneck );
        m_blobView[ header->name ] = tar::data( header );
        auto& b = m_blobs.emplace_back();
        b = std::move( blob );
    }
}
