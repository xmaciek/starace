#pragma once

#include <engine/action.hpp>
#include <engine/async_io.hpp>
#include <engine/fps_limiter.hpp>
#include <engine/fps_meter.hpp>
#include <engine/update_context.hpp>
#include <engine/render_context.hpp>
#include <engine/mouse_event.hpp>
#include <audio/audio.hpp>
#include <renderer/renderer.hpp>
#include <shared/track_allocator.hpp>

#include <SDL_events.h>

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <memory_resource>
#include <tuple>
#include <vector>

struct SDL_Window;

class Engine {
private:
#ifdef TRACY_ENABLE
    [[no_unique_address]]
    TrackAllocator<> m_trackAllocator{};
#endif

    std::atomic<bool> m_isRunning = true;

    FpsLimiter::Mode m_fpsLimiterMode = FpsLimiter::eSleep;
    FpsLimiter::Clock::duration m_targetFrameDuration = std::chrono::nanoseconds( 1'000'000'000ull / 100ull ); // 1s / 100fps

    SDL_Window* m_window = nullptr;
    std::tuple<uint32_t, uint32_t, float> m_viewport{};
    std::unique_ptr<AsyncIO> m_ioPtr{};
    std::unique_ptr<Audio> m_audioPtr{};
    std::unique_ptr<Renderer> m_rendererPtr{};

    std::mutex m_eventsBottleneck{};
    std::pmr::vector<SDL_Event> m_events{};

protected:
    AsyncIO* m_io = nullptr;
    Audio* m_audio = nullptr;
    Renderer* m_renderer = nullptr;
    FPSMeter m_fpsMeter{};

    ~Engine() noexcept;
    Engine( int, char** ) noexcept;

    std::tuple<uint32_t, uint32_t, float> viewport() const;
    void setViewport( uint32_t w, uint32_t h );

    void quit();

    void registerAction( Action::Enum, Actuator );
    void registerAction( Action::Enum, Actuator, Actuator );

    virtual void onActuator( Actuator ) = 0;
    virtual void onInit() = 0;
    virtual void onExit() = 0;
    virtual void onRender( RenderContext ) = 0;
    virtual void onUpdate( const UpdateContext& ) = 0;
    virtual void onMouseEvent( const MouseEvent& ) = 0;
    virtual void onResize( uint32_t, uint32_t ) = 0;

    std::pmr::vector<DisplayMode> displayModes( uint32_t monitor = 0 ) const;
    void setDisplayMode( const DisplayMode& );
    void setTargetFPS( uint32_t, FpsLimiter::Mode );

private:
    void gameThread();
    void processEvents();

    void controllerAdd( int );
    void controllerRemove( int );

public:
    int run();
};
