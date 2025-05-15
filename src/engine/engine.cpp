#include <engine/engine.hpp>

#include "controller_state.hpp"

#include <SDL.h>
#include <profiler.hpp>

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

Engine::Engine( const CreateInfo& ci ) noexcept
{
    ZoneScoped;
    assert( !ci.gameName.empty() );
    m_saveSystem = std::make_unique<SaveSystem>( ci.gameName );
    m_ioPtr = std::make_unique<Filesystem>();
    m_io = m_ioPtr.get();

    {
        ZoneScopedN( "SDL init" );
        SDL_SetHint( SDL_HINT_GAMECONTROLLERCONFIG_FILE, "SDL2_gamecontrollerdb.txt" );

        [[maybe_unused]]
        const int initOK = SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER );
        assert( initOK >= 0 );
    }
    SDL_DisplayMode desktop{};
    SDL_GetDesktopDisplayMode( 0, &desktop );

    {
        ZoneScopedN( "create window" );
        m_window = SDL_CreateWindow( ci.gameName.data()
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            , desktop.w
            , desktop.h
            , Renderer::windowFlag
                | SDL_WINDOW_RESIZABLE
                | SDL_WINDOW_FULLSCREEN_DESKTOP
        );
        assert( m_window );
    }

    const Renderer::CreateInfo rci{
        .window = m_window,
        .vsync = VSync::eOn,
        .gameName = ci.gameName,
        .versionMajor = ci.versionMajor,
        .versionMinor = ci.versionMinor,
        .versionPatch = ci.versionPatch,
    };
    assert( Renderer::create );
    m_renderer = Renderer::create( rci );
    m_rendererPtr = std::unique_ptr<Renderer>( m_renderer );

    m_audio = Audio::create();
    m_audioPtr = std::unique_ptr<Audio>( m_audio );

    setViewport( (uint32_t)desktop.w, (uint32_t)desktop.h ); // SDL does not fire window resize event on windows upon window creation
    setTargetFPS( 200, FpsLimiter::eSpinLock );
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
    using clock = FpsLimiter::Clock;
    clock::duration averagePresentDuration{};
    clock::duration averageSleepOverhead{};
    auto lastUpdate = clock::now();
    float deltaTime = 0.0f;

    while ( m_isRunning.load() ) {
        FrameMark;
        const auto targetFrameEnd = clock::now() + m_targetFrameDuration - averagePresentDuration - averageSleepOverhead;

        processEvents();
        auto now = clock::now();
        const auto dt = std::chrono::duration_cast<std::chrono::nanoseconds>( now - lastUpdate );
        lastUpdate = now;
        deltaTime = (float)dt.count() / 1'000'000'000ull;
        onUpdate( deltaTime );

        m_renderer->beginFrame();
        onRender( m_renderer );
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
        auto presentEnd = clock::now();
        averagePresentDuration += presentEnd - presentBegin;
        averagePresentDuration /= 2;
    }
}

static input::Actuator::Source mapControllerType( SDL_GameControllerType t )
{
    switch ( t ) {
    default:
    case SDL_CONTROLLER_TYPE_XBOX360:
    case SDL_CONTROLLER_TYPE_XBOXONE:
        return input::Actuator::Source::eXBoxOne;

    case SDL_CONTROLLER_TYPE_PS4:
    case SDL_CONTROLLER_TYPE_PS5:
        return input::Actuator::Source::ePS4;
    }
}

void Engine::processEvents()
{
    using namespace input;
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

        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN: {
            MouseEvent mouse{};
            switch ( event.button.button ) {
            case SDL_BUTTON_LEFT: mouse.type = MouseEvent::eClick; break;
            case SDL_BUTTON_RIGHT: mouse.type = MouseEvent::eClickSecondary; break;
            case SDL_BUTTON_MIDDLE: mouse.type = MouseEvent::eClickMiddle; break;
            default: break;
            }
            mouse.value = event.type == SDL_MOUSEBUTTONDOWN ? Actuator::MAX : Actuator::NOMINAL;
            mouse.position = math::vec2{ static_cast<float>( event.button.x ), static_cast<float>( event.button.y ) };
            onMouseEvent( mouse );
        } break;
        case SDL_MOUSEMOTION:
            onMouseEvent( MouseEvent{
                .type = MouseEvent::eMove,
                .position = math::vec2{ event.motion.x, event.motion.y },
            } );
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
            a.source = mapControllerType( SDL_GameControllerTypeForIndex( event.cbutton.which ) );
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
                Actuator ax{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_LEFTX ), x };
                ax.source = mapControllerType( SDL_GameControllerTypeForIndex( event.cbutton.which ) );
                Actuator ay{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_LEFTY ), y };
                ay.source = ax.source;
                if ( x != ox ) onActuator( ax );
                if ( y != oy ) onActuator( ay );
            } continue;

            case SDL_CONTROLLER_AXIS_RIGHTX:
            case SDL_CONTROLLER_AXIS_RIGHTY: {
                int16_t x = state.rx_dzone();
                int16_t y = state.ry_dzone();
                int16_t ox = std::exchange( axes[ SDL_CONTROLLER_AXIS_RIGHTX ], x );
                int16_t oy = std::exchange( axes[ SDL_CONTROLLER_AXIS_RIGHTY ], y );
                Actuator ax{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_RIGHTX ), x };
                ax.source = mapControllerType( SDL_GameControllerTypeForIndex( event.cbutton.which ) );
                Actuator ay{ static_cast<Actuator::Axiscode>( SDL_CONTROLLER_AXIS_RIGHTY ), y };
                ay.source = ax.source;
                if ( x != ox ) onActuator( ax );
                if ( y != oy ) onActuator( ay );
            } continue;

            case SDL_CONTROLLER_AXIS_TRIGGERLEFT: {
                int16_t t = state.lt_dzone();
                int16_t ot = std::exchange( axes[ event.caxis.axis ], t );
                Actuator a{ static_cast<Actuator::Axiscode>( event.caxis.axis ), t };
                a.source = mapControllerType( SDL_GameControllerTypeForIndex( event.caxis.which ) );
                if ( t != ot ) onActuator( a );
            } continue;
            case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: {
                int16_t t = state.rt_dzone();
                int16_t ot = std::exchange( axes[ event.caxis.axis ], t );
                Actuator a{ static_cast<Actuator::Axiscode>( event.caxis.axis ), t };
                a.source = mapControllerType( SDL_GameControllerTypeForIndex( event.caxis.which ) );
                if ( t != ot ) onActuator( a );
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
    assert( w );
    assert( h );
    const float aspect = static_cast<float>( w ) / static_cast<float>( h );
    m_viewport = { w, h, aspect };
}

std::tuple<uint32_t, uint32_t, float> Engine::viewport() const
{
    assert( std::get<0>( m_viewport ) );
    assert( std::get<1>( m_viewport ) );
    assert( std::get<2>( m_viewport ) > 0.0f );
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
