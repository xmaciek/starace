#include <engine/async_io.hpp>

#include <cassert>
#include <fstream>
#include <chrono>

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
    void* ptr = m_pool.alloc();
    assert( ptr && "too many files in loading in queue" );
    Ticket* ticket = new ( ptr ) Ticket( std::filesystem::path{ path }, upstream );
    Ticket* expected = nullptr;
    for ( auto& it : m_pending ) {
        if ( it.compare_exchange_strong( expected, ticket ) ) {
            m_notify.notify_all();
            return;
        }
    }
    assert( !"unable to find free slot" );
}

AsyncIO::Ticket* AsyncIO::next()
{
    for ( auto& it : m_pending ) {
        Ticket* ticket = it.exchange( nullptr );
        if ( ticket ) { return ticket; }
    }
    return nullptr;
}

void AsyncIO::finish( AsyncIO::Ticket* ticket )
{
    for ( auto& it : m_ready ) {
        Ticket* expected = nullptr;
        if ( it.compare_exchange_strong( expected, ticket ) ) {
            return;
        }
    }
    assert( !"cannot finish async load, not enough room" );
}

void AsyncIO::run()
{
    using namespace std::chrono_literals;
    while ( m_isRunning.load() ) {
        m_notify.wait_for( m_uniqueLock, 5ms );
        Ticket* ticket = next();
        if ( !ticket ) { continue; }
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
    std::scoped_lock<std::mutex> sl( m_bottleneck );
    for ( auto& it : m_ready ) {
        Ticket* ticket = it.load();
        if ( !ticket ) { continue; }
        if ( ticket->path != path ) { continue; }
        it.store( nullptr );
        std::pmr::vector<uint8_t> ret = std::move( ticket->data );
        m_pool.dealloc( ticket );
        return ret;
    }
    return {};
}

std::pmr::vector<uint8_t> AsyncIO::getWait( const std::filesystem::path& path )
{
    while ( true ) {
        auto data = get( path );
        if ( data ) {
            return std::move( *data );
        }
    }
}
