#include <engine/filesystem.hpp>
#include <extra/pak.hpp>
#include <platform/utils.hpp>

#include <profiler.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <utility>
#include <iterator>

Filesystem::~Filesystem() noexcept
{
    m_isRunning.store( false );

}

Filesystem::Filesystem() noexcept = default;

std::span<const uint8_t> Filesystem::viewWait( const std::filesystem::path& path )
{
    ZoneScopedN( "Filesystem viewWait" );
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
    ZoneScoped;
    auto size = static_cast<std::streamsize>( data.size() * sizeof( T ) );
    ifs.read( reinterpret_cast<char*>( data.data() ), size );
    return true;
}

void Filesystem::mount( const std::filesystem::path& path )
{
    ZoneScoped;
    if ( path.extension() != ".pak" ) {
        platform::showFatalError( "Data error", "archive not .pak" );
    }
    std::ifstream ifs( path, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) {
        platform::showFatalError( "Data error", "Cannot open .pak file" );
    }

    std::streamsize size = ifs.tellg();
    if ( size > 0xFFFFFFFFu ) {
        platform::showFatalError( "Data corruption error", ".pak file size exceeds size limit" );
    }

    ifs.seekg( 0 );
    pak::Header header{};
    readRaw( ifs, header );
    if ( header.magic != header.MAGIC ) {
        platform::showFatalError( "Data corruption error", ".pak magic field mismatch" );
    };
    if ( (uint64_t)header.offset + header.size > 0xFFFFFFFFu ) {
        platform::showFatalError( "Data corruption error", ".pak entries exceeds file size limit" );
    }
    if ( header.size % 64 ) {
        platform::showFatalError( "Data corruption error", ".pak header::size % 64 != 0" );
    }
    ifs.seekg( 0 );
    using size_type = std::pmr::vector<uint8_t>::size_type;
    std::pmr::vector<uint8_t> blob( static_cast<size_type>( size ) );
    readRaw( ifs, blob );
    ifs.close();

    std::pmr::vector<pak::Entry> entries( header.size / sizeof( pak::Header ) );
    {
        ZoneScopedN( "Headers sort" );
        std::memcpy( entries.data(), blob.data() + header.offset, header.size );
        std::for_each( entries.begin(), entries.end(), [size]( const auto& it )
        {
            if ( (uint64_t)it.offset + it.size > (uint64_t)size ) platform::showFatalError( "Data corruption error", ".pak out of bounds file entry" );
        } );
        std::sort( entries.begin(), entries.end(), [this]( const auto& lhs, const auto& rhs )
        {
            std::string_view l{ std::begin( lhs.name ) };
            std::string_view r{ std::begin( rhs.name ) };
            auto itl = std::find_if( m_callbacks.begin(), m_callbacks.end(), [l]( const auto& a ) { return l.ends_with( a.first ); } );
            auto itr = std::find_if( m_callbacks.begin(), m_callbacks.end(), [r]( const auto& a ) { return r.ends_with( a.first ); } );
            auto dl = std::distance( m_callbacks.begin(), itl );
            auto dr = std::distance( m_callbacks.begin(), itr );
            if ( dl != dr ) return dl < dr;
            return l < r;
        } );
    }

    auto makeSpan = []( const pak::Entry& e, uint8_t* blob )
    {
        return std::span<uint8_t>{ blob + e.offset, blob + e.offset + e.size };
    };

    std::pmr::map<std::filesystem::path, std::span<const uint8_t>> map{};
    for ( auto&& it : entries ) {
        auto& data = ( map[ it.name ] = makeSpan( it, blob.data() ) );
        std::string_view name{ std::begin( it.name ) };
        auto cb = std::find_if( m_callbacks.begin(), m_callbacks.end(), [name]( const auto& a ) { return name.ends_with( a.first ); } );
        if ( cb == m_callbacks.end() ) continue;
        std::invoke( cb->second, name, data );
    }
    std::scoped_lock<std::mutex> sl( m_bottleneck );
    m_blobs.emplace_back() = std::move( blob );
    // NOTE: new entries should overwrite existing entries, but .merge does not do that, hence do the merge the other way
    map.merge( std::move( m_blobView ) );
    m_blobView = std::move( map );
}

void Filesystem::setCallback( std::string_view ext, Callback&& cb )
{
    m_callbacks.emplace_back( std::make_pair( ext, cb ) );
}
