#include <engine/async_io.hpp>
#include <extra/pak.hpp>

#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <utility>

AsyncIO::~AsyncIO() noexcept
{
    m_isRunning.store( false );

}

AsyncIO::AsyncIO() noexcept = default;

std::span<const uint8_t> AsyncIO::viewWait( const std::filesystem::path& path )
{
    ZoneScopedN( "AsyncIO viewWait" );
    while ( m_isRunning.load() ) {
        std::scoped_lock<std::mutex> sl( m_bottleneck );
        auto it = m_blobView.find( path );
        if ( it != m_blobView.end() ) {
            return it->second;
        }
        assert( !"file not found" );
    }

    return {};
}

static bool readRaw( std::ifstream& ifs, pak::Header& h )
{
    ifs.read( reinterpret_cast<char*>( &h ), static_cast<std::streamsize>( sizeof( h ) ) );
    return true;
}

template <typename T>
static bool readRaw( std::ifstream& ifs, std::pmr::vector<T>& data )
{
    auto size = static_cast<std::streamsize>( data.size() * sizeof( T ) );
    ifs.read( reinterpret_cast<char*>( data.data() ), size );
    return true;
}

void AsyncIO::mount( const std::filesystem::path& path )
{
    ZoneScoped;
    if ( path.extension() != ".pak" ) {
        assert( !"archive not .pak" );
        return;
    }
    std::ifstream ifs( path, std::ios::binary | std::ios::ate );
    assert( ifs.is_open() );

    std::streamsize size = ifs.tellg();
    ifs.seekg( 0 );
    pak::Header header{};
    readRaw( ifs, header );
    if ( header.magic != header.MAGIC ) {
        assert( !".pak magic field mismatch" );
        return;
    };
    ifs.seekg( 0 );
    using size_type = std::pmr::vector<uint8_t>::size_type;
    std::pmr::vector<uint8_t> blob( static_cast<size_type>( size ) );
    readRaw( ifs, blob );
    ifs.close();

    std::pmr::vector<pak::Entry> entries( header.size / sizeof( pak::Header ) );
    std::memcpy( entries.data(), blob.data() + header.offset, header.size );


    auto makeSpan = []( const pak::Entry& e, uint8_t* blob )
    {
        return std::span<uint8_t>{ blob + e.offset, blob + e.offset + e.size };
    };

    std::pmr::map<std::filesystem::path, std::span<const uint8_t>> map{};
    for ( auto&& it : entries ) {
        map[ it.name ] = makeSpan( it, blob.data() );
    }
    std::scoped_lock<std::mutex> sl( m_bottleneck );
    m_blobs.emplace_back() = std::move( blob );
    m_blobView.merge( std::move( map ) );
}
