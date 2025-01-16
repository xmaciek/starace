#pragma once

#include <renderer/buffer.hpp>
#include <renderer/display_mode.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <SDL.h>

#include <cstdint>
#include <memory_resource>
#include <vector>
#include <span>

struct DispatchInfo {
    PipelineSlot m_pipeline = 0;
};

class Engine;
class Renderer {
public:
    virtual ~Renderer() noexcept = default;
    Renderer() noexcept = default;

    static Renderer* instance();

    enum class Feature : uint32_t {
        eVSyncMailbox,
    };

    virtual bool featureAvailable( Feature ) const = 0;
    virtual void setVSync( VSync ) = 0;

    [[nodiscard]] virtual PipelineSlot createPipeline( const PipelineCreateInfo& ) = 0;
    [[nodiscard]] virtual Buffer createBuffer( std::span<const float> ) = 0;
    virtual void deleteBuffer( Buffer ) = 0;

    [[nodiscard]] virtual Texture createTexture( const TextureCreateInfo&, std::span<const uint8_t> ) = 0;
    virtual void deleteTexture( Texture ) = 0;

    virtual void push( const PushBuffer&, const void* constant ) = 0;
    virtual void dispatch( const DispatchInfo&, const void* constant ) = 0;

    virtual void setResolution( uint32_t width, uint32_t height ) = 0;

    virtual uint32_t channelCount( Texture ) const = 0;

protected:
    friend class Engine;
    friend class RendererSetup;

    struct CreateInfo{
        SDL_Window* window = nullptr;
        VSync vsync = {};
    };

    using FNCreate = Renderer*(const Renderer::CreateInfo&);
    static inline constinit FNCreate* create = nullptr;
    static inline constinit SDL_WindowFlags windowFlag{};

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void present() = 0;
};
