#pragma once

#include <engine/async_io.hpp>
#include <engine/update_context.hpp>
#include <engine/render_context.hpp>
#include <audio/audio.hpp>
#include <renderer/renderer.hpp>

#include <cstdint>
#include <tuple>


struct SDL_Window;

class Engine {
private:
    SDL_Window* m_window = nullptr;
    std::tuple<uint32_t, uint32_t, float> m_viewport{};
    std::unique_ptr<Renderer> m_rendererPtr{};

protected:
    std::unique_ptr<AsyncIO> m_io{};
    Renderer* m_renderer = nullptr;
    std::unique_ptr<audio::Engine> m_audio{};

    ~Engine() noexcept;
    Engine() noexcept;

    std::tuple<uint32_t, uint32_t, float> viewport() const;
    void setViewport( uint32_t w, uint32_t h );

    virtual void onRender( RenderContext ) = 0;
    virtual void onUpdate( const UpdateContext& ) = 0;
};
