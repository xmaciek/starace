#include <engine/engine.hpp>

#include "controller_state.hpp"

#include <SDL.h>
#include <Tracy.hpp>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <utility>
#include <thread>

static std::array<ControllerState, 4> g_controllers{};
using AxisState = std::array<int16_t, SDL_CONTROLLER_AXIS_MAX>;
static std::array<AxisState, 4> g_axisStates{};
static std::array<bool, SDL_NUM_SCANCODES> g_keyboardState{};


Engine::~Engine() noexcept
{
    ZoneScoped;
    m_audioPtr.reset();
    m_rendererPtr.reset();
    SDL_DestroyWindow( m_window );
    SDL_Quit();
}

Engine::Engine( int, char** ) noexcept
{
    ZoneScoped;
    m_ioPtr = std::make_unique<AsyncIO>();
    m_io = m_ioPtr.get();

    {
        ZoneScopedN( "SDL init" );
        SDL_SetHint( SDL_HINT_GAMECONTROLLERCONFIG_FILE, "SDL2_gamecontrollerdb.txt" );

        [[maybe_unused]]
        const int initOK = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER );
        assert( initOK >= 0 );
    }
    const int width = 1280;
    const int height = 720;
    setViewport( width, height );

    {
        ZoneScopedN( "create window" );
        m_window = SDL_CreateWindow( "Starace"
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            , width
            , height
            , Renderer::windowFlag
                | SDL_WINDOW_RESIZABLE
        );
        assert( m_window );
    }

    assert( Renderer::create );
    m_renderer = Renderer::create( { .window = m_window, .vsync = VSync::eOn } );
    m_rendererPtr = std::unique_ptr<Renderer>( m_renderer );

    m_audio = Audio::create();
    m_audioPtr = std::unique_ptr<Audio>( m_audio );

    setTargetFPS( 144, FpsLimiter::eSpinLock );
}

int Engine::run()
{
    std::thread asyncInit{ &Engine::onInit, this };
    std::thread gameThread{ &Engine::gameThread, this };
    SDL_Event event{};
    while ( m_isRunning.load() ) {
        while ( SDL_PollEvent( &event ) ) {
            std::scoped_lock lock{ m_eventsBottleneck };
            m_events.emplace_back( event );
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }

    auto wait = []( auto& thread ) {
        if ( thread.joinable() ) thread.join();
    };
    wait( asyncInit );
    wait( gameThread );

    onExit();
    return 0;
}

void Engine::gameThread()
{
    UpdateContext updateContext{};
    RenderContext renderContext{
        .renderer = m_renderer,
    };

    using clock = FpsLimiter::Clock;

    clock::duration averagePresentDuration{};
    clock::duration averageSleepOverhead{};
    auto lastUpdate = clock::now();

    while ( m_isRunning.load() ) {
        FrameMark;
        const auto targetFrameEnd = clock::now() + m_targetFrameDuration - averagePresentDuration - averageSleepOverhead;

        m_fpsMeter.frameBegin();

        processEvents();
        auto now = clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>( now - lastUpdate );
        lastUpdate = now;
        updateContext.deltaTime = (float)dt.count() / 1'000'000'000ull;
        onUpdate( updateContext );

        m_renderer->beginFrame();
        onRender( renderContext );
        m_renderer->endFrame();

        {
            ZoneScopedN( "Frame Limiter & Pacer" );
            FpsLimiter::Mode limiterMode = m_fpsLimiterMode;
            now = clock::now();
            while ( now < targetFrameEnd ) {
                switch ( limiterMode ) {
                case FpsLimiter::eOff:
                    now = targetFrameEnd;
                    break;
                case FpsLimiter::eSleep:
                    std::this_thread::sleep_until( targetFrameEnd );
                    now = clock::now();
                    averageSleepOverhead += now - targetFrameEnd;
                    averageSleepOverhead /= 2;
                    now = std::max( targetFrameEnd, now );
                    break;
                case FpsLimiter::eSpinLock:
                    averageSleepOverhead = {};
                    now = clock::now();
                    break;
                }
            }
        }

        auto presentBegin = clock::now();
        m_renderer->present();
        m_fpsMeter.frameEnd();
        auto presentEnd = clock::now();
        averagePresentDuration += presentEnd - presentBegin;
        averagePresentDuration /= 2;
    }
}

void Engine::processEvents()
{

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

        case SDL_WINDOWEVENT:
            switch ( event.window.event ) {
            case SDL_WINDOWEVENT_RESIZED:
                setViewport( static_cast<uint32_t>( event.window.data1 ), static_cast<uint32_t>( event.window.data2 ) );
                onResize( static_cast<uint32_t>( event.window.data1 ), static_cast<uint32_t>( event.window.data2 ) );
                break;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if ( event.button.button == SDL_BUTTON_LEFT ) {
                onMouseEvent( MouseEvent{ MouseEvent::eClick, math::vec2{ event.button.x, event.button.y } } );
            }
            break;
        case SDL_MOUSEMOTION:
            onMouseEvent( MouseEvent{ MouseEvent::eMove, math::vec2{ event.motion.x, event.motion.y } } );
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
            Actuator a{ static_cast<Actuator::Buttoncode>( event.cbutton.button ) };
            a.value = state ? Actuator::MAX : Actuator::NOMINAL;
            onActuator( a );
        } break;

        case SDL_CONTROLLERAXISMOTION:
        {
            auto it = std::find( g_controllers.begin(), g_controllers.end(), static_cast<SDL_JoystickID>( event.caxis.which ) );
            if ( it == g_controllers.end() ) {
                assert( !"controller not found" );
                break;
            }
            auto& state = *it;
            auto& axes = g_axisStates[ (uint32_t)std::distance( g_controllers.begin(), it ) ];

            switch ( event.caxis.axis ) {
            case SDL_CONTROLLER_AXIS_LEFTX: state.lx = event.caxis.value; break;
            case SDL_CONTROLLER_AXIS_LEFTY: state.ly = event.caxis.value; break;
            case SDL_CONTROLLER_AXIS_RIGHTX: state.rx = event.caxis.value; break;
            case SDL_CONTROLLER_AXIS_RIGHTY: state.ry = event.caxis.value; break;
            case SDL_CONTROLLER_AXIS_TRIGGERLEFT: state.lt = event.caxis.value; break;
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: state.rt = event.caxis.value; break;
            }
            switch ( event.caxis.axis ) {
            case SDL_CONTROLLER_AXIS_LEFTX:
            case SDL_CONTROLLER_AXIS_LEFTY: {
                int16_t x = state.lx_dzone();
                int16_t y = state.ly_dzone();
                int16_t ox = std::exchange( axes[ SDL_CONTROLLER_AXIS_LEFTX ], x );
                int16_t oy = std::exchange( axes[ SDL_CONTROLLER_AXIS_LEFTY ], y );
                if ( x != ox ) onActuator( Actuator{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_LEFTX ), x } );
                if ( y != oy ) onActuator( Actuator{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_LEFTY ), y } );
            } continue;

            case SDL_CONTROLLER_AXIS_RIGHTX:
            case SDL_CONTROLLER_AXIS_RIGHTY: {
                int16_t x = state.rx_dzone();
                int16_t y = state.ry_dzone();
                int16_t ox = std::exchange( axes[ SDL_CONTROLLER_AXIS_RIGHTX ], x );
                int16_t oy = std::exchange( axes[ SDL_CONTROLLER_AXIS_RIGHTY ], y );
                if ( x != ox ) onActuator( Actuator{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_RIGHTX ), x } );
                if ( y != oy ) onActuator( Actuator{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_RIGHTY ), y } );
            } continue;

            case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
                int16_t t = state.lt_dzone();
                int16_t ot = std::exchange( axes[ event.caxis.axis ], t );
                if ( t != ot ) onActuator( Actuator{ static_cast<Actuator::Axiscode>( event.caxis.axis ), t } );
            } continue;
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
                int16_t t = state.rt_dzone();
                int16_t ot = std::exchange( axes[ event.caxis.axis ], t );
                if ( t != ot ) onActuator( Actuator{ static_cast<Actuator::Axiscode>( event.caxis.axis ), t } );
            } continue;
            }

        } break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            const bool newState = event.key.state == SDL_PRESSED;
            const bool oldState = std::exchange( g_keyboardState[ event.key.keysym.scancode ], newState );
            if ( oldState == newState ) {
                break;
            }

            Actuator a{ static_cast<Actuator::Scancode>( event.key.keysym.scancode ) };
            a.value = newState ? Actuator::MAX : Actuator::NOMINAL;
            onActuator( a );
        } break;

        default:
            break;
        }
    }
}

void Engine::quit()
{
    m_isRunning.store( false );
}

void Engine::setTargetFPS( uint32_t fps, FpsLimiter::Mode mode )
{
    m_targetFrameDuration = std::chrono::nanoseconds( 1'000'000'000ull / fps );
    m_fpsLimiterMode = mode;
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

void Engine::controllerAdd( int idx )
{
    if ( SDL_FALSE == SDL_GetHintBoolean( "SDL_GAMECONTROLLER_ALLOW_STEAM_VIRTUAL_GAMEPAD", SDL_FALSE ) ) {
        const char* name = SDL_GameControllerNameForIndex( idx );
        const std::string_view namesv = name ? name : "";
        if ( namesv == "Steam Virtual Gamepad" ) {
            return;
        }
    }
    if ( SDL_GameControllerTypeForIndex( idx ) == SDL_CONTROLLER_TYPE_VIRTUAL ) {
        return;
    }
    auto it = std::find( g_controllers.begin(), g_controllers.end(), nullptr );
    if ( it == g_controllers.end() ) {
        // controllers full
        return;
    }

    it->controller = SDL_GameControllerOpen( idx );
    it->id = SDL_JoystickGetDeviceInstanceID( idx );
    assert( it->controller );
    assert( it->id > -1 );
}

void Engine::controllerRemove( int id )
{
    auto it = std::find( g_controllers.begin(), g_controllers.end(), static_cast<SDL_JoystickID>( id ) );
    if ( it == g_controllers.end() ) {
        assert( !"controller to remove not found" );
        return;
    }
    g_axisStates[ (uint32_t)std::distance( g_controllers.begin(), it ) ] = {};
    auto controller = it->controller;
    *it = {};
    SDL_GameControllerClose( controller );
}

static std::pmr::vector<SDL_DisplayMode> getDisplayModes( uint32_t monitor )
{
    ZoneScoped;
    const int displayModeCount = SDL_GetNumDisplayModes( static_cast<int>( monitor ) );
    if ( displayModeCount < 1 ) {
        return {};
    }

    std::pmr::vector<SDL_DisplayMode> ret( static_cast<size_t>( displayModeCount ) );

    auto genMode = [ i = 0, monitor ]() mutable
    {
        SDL_DisplayMode mode{};
        SDL_GetDisplayMode( static_cast<int>( monitor ), i++, &mode );
        return mode;
    };
    std::generate_n( ret.begin(), displayModeCount, genMode );

    auto eq = []( const auto& lhs, const auto& rhs )
    {
        return lhs.w == rhs.w && lhs.h == rhs.h;
    };
    auto last = std::unique( ret.begin(), ret.end(), eq );
    if ( last != ret.end() ) ret.erase( last, ret.end() );

    return ret;
}

std::pmr::vector<DisplayMode> Engine::displayModes( uint32_t monitor ) const
{
    ZoneScoped;
    auto modes = getDisplayModes( monitor );
    std::pmr::vector<DisplayMode> ret{ modes.size() };
    std::transform( modes.begin(), modes.end(), ret.begin(), [monitor]( const auto& it ) -> DisplayMode
    {
        return {
            .width = static_cast<uint16_t>( it.w ),
            .height = static_cast<uint16_t>( it.h ),
            .monitor = static_cast<uint16_t>( monitor ),
        };
    } );
    return ret;
}

void Engine::setDisplayMode( const DisplayMode& displayMode )
{
    ZoneScoped;
    auto modes = getDisplayModes( displayMode.monitor );
    auto cmp = [ displayMode ]( const auto& a, const auto& b )
    {
        const int w1 = static_cast<int>( displayMode.width ) - a.w;
        const int w2 = static_cast<int>( displayMode.width ) - b.w;
        if ( w1 != w2 ) { return std::abs( w1 ) < std::abs( w2 ); }

        const int h1 = static_cast<int>( displayMode.height ) - a.h;
        const int h2 = static_cast<int>( displayMode.height ) - b.h;
        return std::abs( h1 ) < std::abs( h2 );
    };

    auto it = std::min_element( modes.begin(), modes.end(), cmp );
    assert( it != modes.end() );

    const SDL_DisplayMode nearestMode = *it;
    m_renderer->setResolution( static_cast<uint32_t>( nearestMode.w ), static_cast<uint32_t>( nearestMode.h ) );

    SDL_Event e{};
    e.type = SDL_WINDOWEVENT;
    e.window.event = SDL_WINDOWEVENT_RESIZED;

    if ( displayMode.fullscreen ) {
        SDL_DisplayMode desktopMode{};
        SDL_GetDesktopDisplayMode( displayMode.monitor, &desktopMode );
        SDL_SetWindowFullscreen( m_window, SDL_WINDOW_FULLSCREEN_DESKTOP );
        e.window.data1 = desktopMode.w;
        e.window.data2 = desktopMode.h;
    }
    else {
        SDL_SetWindowFullscreen( m_window, 0 );
        SDL_SetWindowSize( m_window, nearestMode.w, nearestMode.h );
        SDL_SetWindowPosition( m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
        e.window.data1 = nearestMode.w;
        e.window.data2 = nearestMode.h;
    }

    SDL_PushEvent( &e );

}
