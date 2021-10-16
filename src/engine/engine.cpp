#include <engine/engine.hpp>

#include <SDL2/SDL.h>

#include <cstring>
#include <fstream>
#include <utility>

Engine::~Engine() noexcept
{
    SDL_DestroyWindow( m_window );
    SDL_Quit();
}


struct BinaryConfigBlob {
    std::pair<uint32_t, uint32_t> windowExtent{ 1280, 720 };
};

static BinaryConfigBlob loadConfig()
{
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


Engine::Engine() noexcept
: m_io{ std::make_unique<AsyncIO>() }
{
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

void Engine::setViewport( uint32_t w, uint32_t h )
{
    const float aspect = static_cast<float>( w ) / static_cast<float>( h );
    m_viewport = { w, h, aspect };
}

std::tuple<uint32_t, uint32_t, float> Engine::viewport() const
{
    return m_viewport;
}
