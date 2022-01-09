#include <engine/engine.hpp>

#include "action_mapping.hpp"

#include <SDL2/SDL.h>
#include <Tracy.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <utility>
#include <iostream>

Engine::~Engine() noexcept
{
    ZoneScoped;
    m_audioPtr.reset();
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
{
    ZoneScoped;
    {
        ZoneScopedN( "SDL init" );
        [[maybe_unused]]
        const int initOK = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER );
        assert( initOK >= 0 );
    }

    const BinaryConfigBlob blob = loadConfig();
    setViewport( blob.windowExtent.first, blob.windowExtent.second );

    {
        ZoneScopedN( "create window" );
        m_window = SDL_CreateWindow( "Starace"
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            , static_cast<int>( blob.windowExtent.first )
            , static_cast<int>( blob.windowExtent.second )
            , Renderer::windowFlag()
                | SDL_WINDOW_RESIZABLE
        );
        assert( m_window );
    }
    m_actionMapping = std::make_unique<ActionMapping>();
    m_ioPtr = std::make_unique<AsyncIO>();
    m_io = m_ioPtr.get();

    m_renderer = Renderer::create( m_window );
    m_rendererPtr = std::unique_ptr<Renderer>( m_renderer );

    m_audio = Audio::create();
    m_audioPtr = std::unique_ptr<Audio>( m_audio );

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
        processEvents();
        onUpdate( updateContext );
        m_renderer->beginFrame();
        onRender( renderContext );
        m_renderer->endFrame();
        m_fpsMeter.frameEnd();

        m_renderer->present();

        const std::chrono::time_point now = std::chrono::steady_clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::microseconds>( now - tp );
        updateContext.deltaTime = (float)dt.count() / 1'000'000;
    }
}

void Engine::processEvents()
{

    static constexpr auto clampDeadZone = [] ( int16_t value ) -> std::tuple<int16_t,float>
    {
        static constexpr int16_t dzone = 8000;
        const int16_t sub = std::clamp<short>( value, -dzone, dzone );
        const int16_t div = SDL_JOYSTICK_AXIS_MAX - dzone;
        const float norm = static_cast<float>( value - sub ) / static_cast<float>( div );
        return { static_cast<int16_t>( norm * SDL_JOYSTICK_AXIS_MAX ), norm };
    };

    std::pmr::vector<SDL_Event> events{};
    {
        std::scoped_lock lock{ m_eventsBottleneck };
        std::swap( events, m_events );
    }

    for ( SDL_Event& event : events ) {
        switch ( event.type ) {
        case SDL_QUIT:
            quit();
            return;
        case SDL_MOUSEBUTTONDOWN:
            if ( event.button.button == SDL_BUTTON_LEFT ) {
                onMouseEvent( MouseClick{ glm::vec2{ event.button.x, event.button.y } } );
            }
            break;
        case SDL_MOUSEMOTION:
            onMouseEvent( MouseMove{ glm::vec2{ event.motion.x, event.motion.y } } );
            break;

        case SDL_CONTROLLERDEVICEADDED:
            controllerAdd( event.cdevice.which );
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            controllerRemove( event.cdevice.which );
            break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
        {
            const bool state = event.cbutton.state == SDL_PRESSED;
            const Actuator a{ static_cast<Actuator::Buttoncode>( event.cbutton.button ) };
            auto [ it, end ] = m_actionMapping->resolve1( a );
            for ( ; it != end; ++it ) {
                assert( it );
                onAction( Action{ .digital = state, .userEnum = *it } );
            }
        } break;

        case SDL_CONTROLLERAXISMOTION:
        {
            [[maybe_unused]]
            static auto tid = std::this_thread::get_id();
            assert( tid == std::this_thread::get_id() );
            thread_local std::array<int16_t, SDL_CONTROLLER_AXIS_MAX> axisState{};

            const auto [ newState, newStateF ] = clampDeadZone( event.caxis.value );
            const int16_t oldState = std::exchange( axisState[ event.caxis.axis ], newState );
            if ( oldState == newState ) {
                break;
            }

            const Actuator a{ static_cast<Actuator::Axiscode>( event.caxis.axis ) };
            auto [ it, end ] = m_actionMapping->resolve1( a );
            for ( ; it != end; ++it ) {
                assert( it );
                onAction( Action{ .analog = newStateF, .userEnum = *it } );
            }
            auto actions = m_actionMapping->resolve2( a, newState );
            for ( auto [ eid, value ] : actions ) {
                onAction( Action{ .analog = value, .userEnum = eid } );
            }
        } break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            [[maybe_unused]]
            static auto tid = std::this_thread::get_id();
            assert( tid == std::this_thread::get_id() );
            thread_local std::array<bool, SDL_NUM_SCANCODES> keyboardState{};
            const bool newState = event.key.state == SDL_PRESSED;
            const bool oldState = std::exchange( keyboardState[ event.key.keysym.scancode ], newState );
            if ( oldState == newState ) {
                break;
            }

            const Actuator a{ static_cast<Actuator::Scancode>( event.key.keysym.scancode ) };
            auto [ it, end ] = m_actionMapping->resolve1( a );
            for ( ; it != end; ++it ) {
                assert( it );
                onAction( Action{ .digital = newState, .userEnum = *it } );
            }
            auto actions = m_actionMapping->resolve2( a, newState );
            for ( auto [ eid, value ] : actions ) {
                onAction( Action{ .analog = value, .userEnum = eid } );
            }
        } break;

        default:
            onEvent( event );
            break;
        }
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

void Engine::registerAction( Action::Enum eid, Actuator a )
{
    m_actionMapping->registerAction( eid, a );
}

void Engine::registerAction( Action::Enum eid, Actuator a, Actuator b )
{
    m_actionMapping->registerAction( eid, a, b );
}

void Engine::controllerAdd( int idx )
{
    if ( SDL_FALSE == SDL_GetHintBoolean( "SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD", SDL_FALSE ) ) {
        const char* name = SDL_GameControllerNameForIndex( idx );
        const std::string_view namesv = name ? name : "";
        if ( namesv == "Steam Virtual Gamepad" ) {
            return;
        }
    }
    SDL_GameController* controller = SDL_GameControllerOpen( idx );
    assert( controller );
    m_controllers.emplace_back( controller );
}

void Engine::controllerRemove( int id )
{
    SDL_GameController* controller = SDL_GameControllerFromInstanceID( id );
    std::erase( m_controllers, controller );
    SDL_GameControllerClose( controller );
}
