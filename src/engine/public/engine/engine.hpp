#pragma once

#include <engine/async_io.hpp>
#include <engine/fps_meter.hpp>
#include <engine/update_context.hpp>
#include <engine/render_context.hpp>
#include <audio/audio.hpp>
#include <renderer/renderer.hpp>

#include <SDL2/SDL_events.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <memory_resource>
#include <tuple>
#include <vector>

struct SDL_Window;

class Engine {
private:
    std::atomic<bool> m_isRunning = true;

    SDL_Window* m_window = nullptr;
    std::tuple<uint32_t, uint32_t, float> m_viewport{};
    std::unique_ptr<Renderer> m_rendererPtr{};

    std::mutex m_eventsBottleneck{};
    std::pmr::vector<SDL_Event> m_events{};

    void gameThread();

protected:
    std::unique_ptr<AsyncIO> m_io{};
    Renderer* m_renderer = nullptr;
    std::unique_ptr<audio::Engine> m_audio{};
    FPSMeter m_fpsMeter{};

    ~Engine() noexcept;
    Engine() noexcept;

    std::tuple<uint32_t, uint32_t, float> viewport() const;
    void setViewport( uint32_t w, uint32_t h );

    void quit();
    virtual void onInit() = 0;
    virtual void onExit() = 0;
    virtual void onRender( RenderContext ) = 0;
    virtual void onUpdate( const UpdateContext& ) = 0;
    virtual void onEvent( const SDL_Event& ) = 0;

public:
    int run();
};
