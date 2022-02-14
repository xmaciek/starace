#include <engine/async_io.hpp>

#include <Tracy.hpp>

#include <cassert>
#include <fstream>
#include <chrono>
#include <iostream>

AsyncIO::Ticket::Ticket( std::filesystem::path p, std::pmr::memory_resource* upstream )
: path{ std::move( p ) }
, data{ upstream }
{}

AsyncIO::~AsyncIO() noexcept
{
    m_isRunning.store( false );
    if ( m_thread.joinable() ) {
        m_thread.join();
    }
}

AsyncIO::AsyncIO() noexcept
{
    m_thread = std::thread{ &AsyncIO::run, this };
}

void AsyncIO::enqueue( const std::filesystem::path& path, std::pmr::memory_resource* upstream )
{
    ZoneScoped;
    auto it = m_localFiles.find( path );
    assert( it != m_localFiles.end() );
    assert( !it->second );

    void* ptr = m_pool.alloc();
    assert( ptr && "too many files in loading in queue" );
    Ticket* ticket = new ( ptr ) Ticket( path, upstream );
    it->second = ticket;
    {
        std::scoped_lock<std::mutex> sl( m_bottleneck );
        m_pending.push_back( ticket );
    }
    m_notify.release();
}

AsyncIO::Ticket* AsyncIO::next()
{
    std::scoped_lock<std::mutex> sl( m_bottleneck );
    if ( m_pending.empty() ) {
        return nullptr;
    }
    Ticket* t = m_pending.front();
    m_pending.pop_front();
    return t;
}

void AsyncIO::finish( AsyncIO::Ticket* ticket )
{
    assert( ticket );
    ticket->ready.store( true );
}

void AsyncIO::run()
{
    using namespace std::chrono_literals;
    while ( m_isRunning.load() ) {
        if ( !m_notify.try_acquire_for( 5ms ) ) {
            continue;
        }

        Ticket* ticket = next();
        if ( !ticket ) { continue; }

        ZoneScopedN( "AsyncIO load file" );
        std::ifstream ifs( ticket->path, std::ios::binary | std::ios::ate );
        if ( !ifs.is_open() ) {
            finish( ticket );
            continue;
        }

        const std::streamsize size = ifs.tellg();
        ticket->data.resize( size );
        ifs.seekg( 0 );
        ifs.read( reinterpret_cast<char*>( ticket->data.data() ), size );
        finish( ticket );
    }
}

std::optional<std::pmr::vector<uint8_t>> AsyncIO::get( const std::filesystem::path& path )
{
    auto it = m_localFiles.find( path );
    assert( it != m_localFiles.end() );
    assert( it->second );
    if ( !it->second->ready.load() ) { return {}; }

    Ticket* ticket = std::exchange( it->second, nullptr );
    std::pmr::vector<uint8_t> ret = std::move( ticket->data );
    std::destroy_at<Ticket>( ticket );
    m_pool.dealloc( ticket );
    return ret;

}

std::pmr::vector<uint8_t> AsyncIO::getWait( const std::filesystem::path& path )
{
    ZoneScoped;
    while ( m_isRunning.load() ) {
        auto data = get( path );
        if ( data ) {
            return std::move( *data );
        }
    }
    return {};
}

void AsyncIO::mount( const std::filesystem::path& path )
{
    ZoneScoped;
    assert( std::filesystem::is_directory( path ) ); // for now only directories
    constexpr std::array extensionList = {
        ".wav", ".spv", ".tga", ".objc", ".ttf",
    };
    std::pmr::vector<std::filesystem::path> files{};
    files.reserve( 20 );
    for ( auto it : std::filesystem::recursive_directory_iterator( path ) ) {
        if ( it.is_directory() ) { continue; }
        if ( std::none_of( extensionList.begin(), extensionList.end(), [&it]( const auto& e ) { return it.path().extension() == e; } ) ) { continue; }
        files.push_back( it.path() );
    }

    for ( const auto& path : files ) {
        m_localFiles.insert( std::make_pair( path.lexically_relative( "." ), nullptr ) );
    }
}
