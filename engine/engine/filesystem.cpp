#include <engine/filesystem.hpp>
#include <extra/pak.hpp>
#include <platform/utils.hpp>

#include <profiler.hpp>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <utility>

Filesystem::~Filesystem() noexcept = default;

Filesystem::Filesystem() noexcept = default;

std::span<const uint8_t> Filesystem::viewWait( std::string_view path )
{
    ZoneScopedN( "Filesystem viewWait" );
    std::pmr::string key( path );
    std::scoped_lock<std::mutex> sl( m_bottleneckFs );
    for ( auto&& mount : m_mounts ) {
        auto it = mount.m_toc.find( key );
        if ( it != mount.m_toc.end() ) { return it->second; }
    }
    assert( !"file not found" );
    return {};
}

static void readRaw( std::ifstream& ifs, pak::Header& h )
{
    ifs.read( reinterpret_cast<char*>( &h ), static_cast<std::streamsize>( sizeof( h ) ) );
}

template <typename T>
static void readRaw( std::ifstream& ifs, std::span<T> data )
{
    ZoneScoped;
    auto size = static_cast<std::streamsize>( data.size() * sizeof( T ) );
    ifs.read( reinterpret_cast<char*>( data.data() ), size );
}

static inline auto align16( auto v )
{
    return (decltype(v))( ( (uintptr_t)v + 15ull ) & ~15ull );
};

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

    using size_type = decltype( Mount::m_blob )::size_type;
    static constexpr size_type MAX_SIZE = 0xFFFF'FFFFu;
    const size_type fileSize = static_cast<size_type>( ifs.tellg() );
    if ( fileSize > MAX_SIZE ) {
        platform::showFatalError( "Data corruption error", ".pak file size exceeds size limit" );
    }

    ifs.seekg( 0 );
    pak::Header header{};
    readRaw( ifs, header );
    if ( header.magic != header.MAGIC ) {
        platform::showFatalError( "Data corruption error", ".pak magic field mismatch" );
    };
    if ( ( (size_type)header.offset + header.size ) > MAX_SIZE ) {
        platform::showFatalError( "Data corruption error", ".pak entries exceeds file size limit" );
    }
    if ( header.size % 64 ) {
        platform::showFatalError( "Data corruption error", ".pak header::size % 64 != 0" );
    }

    std::pmr::vector<pak::Entry> entries( header.size / sizeof( pak::Entry ) );
    assert( !entries.empty() );
    size_type preallocSize = 0;
    {
        ZoneScopedN( "read & process .pak TOC" );
        ifs.seekg( header.offset );
        readRaw( ifs, std::span<pak::Entry>( entries ) );

        std::ranges::for_each( entries, [fileSize, &preallocSize]( const auto& it )
        {
            if ( (size_type)it.offset + it.size > fileSize ) [[unlikely]] {
                platform::showFatalError( "Data corruption error", ".pak out of bounds file entry" );
            }
            if ( it.offset < sizeof( header ) ) [[unlikely]]
            {
                platform::showFatalError( "Data corruption error", ".pak entry overwrites pak header" );
            }
            // NOTE: align file sizes by 16 for easier debugging
            preallocSize += align16( it.size );
        } );
        assert( preallocSize != 0 );
        assert( preallocSize <= MAX_SIZE );
        std::ranges::sort( entries, [this]( const auto& lhs, const auto& rhs )
        {
            std::string_view l{ std::begin( lhs.name ) };
            std::string_view r{ std::begin( rhs.name ) };
            auto itl = std::ranges::find_if( m_callbacks, [l]( const auto& a ) { return l.ends_with( a.first ); } );
            auto itr = std::ranges::find_if( m_callbacks, [r]( const auto& a ) { return r.ends_with( a.first ); } );
            auto dl = std::distance( m_callbacks.begin(), itl );
            auto dr = std::distance( m_callbacks.begin(), itr );
            if ( dl != dr ) return dl < dr;
            return l < r;
        } );
    }

    // NOTE: I have no guarantees default allocator will give me 16 byte aligned pointer
    decltype( Mount::m_blob ) blob( preallocSize + 16 );
    decltype( Mount::m_toc ) map;
    auto* ptr = blob.data();
    std::ranges::for_each( entries, [this, &ifs, &map, &ptr]( const auto& entry )
    {
        std::string_view name{ std::begin( entry.name ) };
        ptr = align16( ptr );
        ifs.seekg( entry.offset );
        [[maybe_unused]]
        auto [ it, inserted ] = map.insert( std::make_pair( name, std::span<uint8_t>{ ptr, entry.size } ) );
        assert( inserted );
        ptr += entry.size;
        readRaw( ifs, it->second );

        std::scoped_lock sl{ m_bottleneckCb };
        auto cb = std::ranges::find_if( m_callbacks, [name]( const auto& a ) { return name.ends_with( a.first ); } );
        if ( cb == m_callbacks.end() ) return;
        // TODO: move to async once file dependency is solved
        std::invoke( cb->second, Asset{ name, it->second } );
    } );

    std::scoped_lock sl{ m_bottleneckFs };
    auto& mount = m_mounts.emplace_front();
    mount.m_blob = std::move( blob );
    mount.m_toc = std::move( map );
}

void Filesystem::setCallback( std::string_view ext, Callback&& cb )
{
    std::scoped_lock sl{ m_bottleneckFs };
    m_callbacks.emplace_back( std::make_pair( ext, cb ) );
}
