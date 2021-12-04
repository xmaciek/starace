#include <engine/engine.hpp>

#include <SDL2/SDL.h>
#include <Tracy.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <utility>

Engine::~Engine() noexcept
{
    ZoneScoped;
    m_rendererPtr.reset();
    SDL_DestroyWindow( m_window );
    SDL_Quit();
}


struct BinaryConfigBlob {
    std::pair<uint32_t, uint32_t> windowExtent{ 1280, 720 };
};

static BinaryConfigBlob loadConfig()
{
    ZoneScoped;
    BinaryConfigBlob ret{};
    std::ifstream file( "engine.cfg" );
    std::array<char, 48> tmp1{};
    std::array<char, 48> tmp2{};

    std::pmr::string line;
    while ( getline( file, line ) ) {
        std::sscanf( line.c_str(), "%s %s", tmp1.data(), tmp2.data() );
        if ( std::strcmp( tmp1.data(), "windowWidth" ) == 0 ) {
            ret.windowExtent.first = std::atoi( tmp2.data() );
        }
        else if ( std::strcmp( tmp1.data(), "windowHeight" ) == 0 ) {
            ret.windowExtent.second = std::atoi( tmp2.data() );
        }
    }
    return ret;
}


Engine::Engine( int, char** ) noexcept
: m_io{ std::make_unique<AsyncIO>() }
{
    ZoneScoped;
    [[maybe_unused]]
    const int initOK = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER );
    assert( initOK >= 0 );

    const BinaryConfigBlob blob = loadConfig();
    setViewport( blob.windowExtent.first, blob.windowExtent.second );

    m_window = SDL_CreateWindow( "Starace"
        , SDL_WINDOWPOS_CENTERED
        , SDL_WINDOWPOS_CENTERED
        , static_cast<int>( blob.windowExtent.first )
        , static_cast<int>( blob.windowExtent.second )
        , Renderer::windowFlag()
            | SDL_WINDOW_RESIZABLE
    );
    assert( m_window );

    m_renderer = Renderer::create( m_window );
    m_rendererPtr = std::unique_ptr<Renderer>( m_renderer );
    assert( m_renderer );

    m_audio = std::unique_ptr<audio::Engine>( audio::Engine::create() );
    assert( m_audio );

}

int Engine::run()
{
    onInit();
    std::thread gameThread{ &Engine::gameThread, this };
    SDL_Event event{};
    while ( m_isRunning.load() ) {
        while ( SDL_PollEvent( &event ) ) {
            std::scoped_lock lock{ m_eventsBottleneck };
            m_events.emplace_back( event );
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }

    if ( gameThread.joinable() ) {
        gameThread.join();
    }

    onExit();
    return 0;
}

void Engine::gameThread()
{
    UpdateContext updateContext{ .deltaTime = 0.0166f };
    RenderContext renderContext{
        .renderer = m_renderer,
    };

    while ( m_isRunning.load() ) {
        FrameMark;
        const std::chrono::time_point tp = std::chrono::steady_clock::now();

        m_fpsMeter.frameBegin();
        onUpdate( updateContext );
        m_renderer->beginFrame();
        onRender( renderContext );
        m_renderer->endFrame();

        std::pmr::vector<SDL_Event> events{};
        {
            std::scoped_lock lock{ m_eventsBottleneck };
            std::swap( events, m_events );
        }
        for ( SDL_Event& it : events ) {
            onEvent( it );
        }
        m_fpsMeter.frameEnd();
        m_renderer->present();

        const std::chrono::time_point now = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::microseconds>( now - tp );
        updateContext.deltaTime = (float)dt.count();
        // TODO: fix game speed later
        updateContext.deltaTime /= 500'000;
    }
}

void Engine::quit()
{
    m_isRunning.store( false );
}

void Engine::setViewport( uint32_t w, uint32_t h )
{
    const float aspect = static_cast<float>( w ) / static_cast<float>( h );
    m_viewport = { w, h, aspect };
}

std::tuple<uint32_t, uint32_t, float> Engine::viewport() const
{
    return m_viewport;
}
