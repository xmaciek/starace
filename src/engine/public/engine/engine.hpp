#pragma once

#include <engine/action.hpp>
#include <engine/action_mapping.hpp>
#include <engine/async_io.hpp>
#include <engine/fps_meter.hpp>
#include <engine/update_context.hpp>
#include <engine/render_context.hpp>
#include <engine/mouse_event.hpp>
#include <audio/audio.hpp>
#include <renderer/renderer.hpp>

#include <SDL2/SDL_events.h>

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
    std::atomic<bool> m_isRunning = true;

    SDL_Window* m_window = nullptr;
    std::tuple<uint32_t, uint32_t, float> m_viewport{};
    std::unique_ptr<Renderer> m_rendererPtr{};
    std::unique_ptr<Audio> m_audioPtr{};

    std::mutex m_eventsBottleneck{};
    std::pmr::vector<SDL_Event> m_events{};

    ActionMapping m_actionMapping{};
    std::pmr::vector<SDL_GameController*> m_controllers{};

protected:
    std::unique_ptr<AsyncIO> m_io{};
    Renderer* m_renderer = nullptr;
    Audio* m_audio = nullptr;
    FPSMeter m_fpsMeter{};

    ~Engine() noexcept;
    Engine( int, char** ) noexcept;

    std::tuple<uint32_t, uint32_t, float> viewport() const;
    void setViewport( uint32_t w, uint32_t h );

    void quit();

    void registerAction( Action::Enum, Actuator );
    void registerAction( Action::Enum, Actuator, Actuator );

    virtual void onAction( Action ) = 0;
    virtual void onInit() = 0;
    virtual void onExit() = 0;
    virtual void onRender( RenderContext ) = 0;
    virtual void onUpdate( const UpdateContext& ) = 0;
    virtual void onEvent( const SDL_Event& ) = 0;
    virtual void onMouseEvent( const MouseEvent& ) = 0;

private:
    void gameThread();
    void processEvents();

    void controllerAdd( int );
    void controllerRemove( int );

public:
    int run();
};
