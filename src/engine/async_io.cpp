#include <engine/async_io.hpp>

#include <Tracy.hpp>

#include <cassert>
#include <fstream>
#include <chrono>
#include <iostream>

AsyncIO::Ticket::Ticket( std::filesystem::path&& path, std::pmr::memory_resource* upstream )
: path{ std::move( path ) }
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
: m_uniqueLock{ m_mutex }
{
    m_thread = std::thread{ &AsyncIO::run, this };
}

void AsyncIO::enqueue( const std::filesystem::path& path, std::pmr::memory_resource* upstream )
{
    ZoneScoped;
    void* ptr = m_pool.alloc();
    assert( ptr && "too many files in loading in queue" );
    Ticket* ticket = new ( ptr ) Ticket( std::filesystem::path{ path }, upstream );
    {
        std::scoped_lock<std::mutex> sl( m_bottleneck );
        m_pending.push_back( ticket );
    }
    m_pendingCount.fetch_add( 1 );
    m_notify.notify_all();
}

AsyncIO::Ticket* AsyncIO::next()
{
    if ( m_pendingCount.load() == 0 ) {
        return nullptr;
    }
    std::scoped_lock<std::mutex> sl( m_bottleneck );
    if ( m_pending.empty() ) {
        return nullptr;
    }
    assert( m_pendingCount.load() != 0 );
    m_pendingCount.fetch_sub( 1 );
    AsyncIO::Ticket* t = m_pending.front();
    m_pending.pop_front();
    return t;
}

void AsyncIO::finish( AsyncIO::Ticket* ticket )
{
    std::scoped_lock<std::mutex> sl( m_bottleneckReady );
    m_ready.push_back( ticket );
}

void AsyncIO::run()
{
    using namespace std::chrono_literals;
    while ( m_isRunning.load() ) {
        if ( m_pendingCount.load() == 0 ) {
            m_notify.wait_for( m_uniqueLock, 5ms );
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
    std::scoped_lock<std::mutex> sl( m_bottleneckReady );
    if ( m_ready.empty() ) { return {}; }

    ZoneScoped;
    for ( auto it = m_ready.begin(); it != m_ready.end(); ++it ) {
        Ticket* ticket = *it;
        assert( ticket );
        if ( ticket->path != path ) {
            std::cout << path.native() << " != " << ticket->path.native() << std::endl;
            continue; }

        m_ready.erase( it );
        std::pmr::vector<uint8_t> ret = std::move( ticket->data );
        std::destroy_at<Ticket>( ticket );
        m_pool.dealloc( ticket );
        return ret;
    }
    return {};
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
