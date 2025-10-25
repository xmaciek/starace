#pragma once

#include <renderer/buffer.hpp>
#include <renderer/display_mode.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>

#include <SDL.h>

#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <vector>
#include <span>
#include <string_view>

struct DispatchInfo {
    struct Uniform {
        const void* ptr{};
        size_t size{};
        inline Uniform() = default;
        inline Uniform( const Uniform& ) = default;
        inline Uniform& operator = ( const Uniform& ) = default;
        inline Uniform( const auto& u ) : ptr{ &u }, size{ sizeof( u ) } {}
    };
    PipelineSlot m_pipeline = 0;
    Uniform m_uniform{};
};

struct RenderInfo {
    struct Uniform {
        const void* ptr{};
        size_t size{};
        inline Uniform() = default;
        inline Uniform( const Uniform& ) = default;
        inline Uniform& operator = ( const Uniform& ) = default;
        inline Uniform( const auto& u ) : ptr{ &u }, size{ sizeof( u ) } {}
    };
    enum : uint32_t {
        MAX_TEXTURES = 9,
    };
    PipelineSlot m_pipeline{};
    uint32_t m_verticeCount = 0;
    uint32_t m_instanceCount = 1;
    float m_lineWidth = 1.0f;
    Uniform m_uniform{};
    Buffer m_vertexBuffer{};
    std::array<Texture, MAX_TEXTURES> m_fragmentTexture{};
};

class Engine;
class Renderer {
public:
    virtual ~Renderer() noexcept = default;
    Renderer() noexcept = default;

    static Renderer* instance() { return s_instance; }

    enum class Feature : uint32_t {
        eVSyncMailbox,
        eVRSAA,
    };

    virtual bool featureAvailable( Feature ) const = 0;
    virtual void setFeatureEnabled( Feature, bool ) = 0;
    virtual void setVSync( VSync ) = 0;

    [[nodiscard]] virtual PipelineSlot createPipeline( const PipelineCreateInfo& ) = 0;
    [[nodiscard]] virtual Buffer createBuffer( std::span<const uint8_t> ) = 0;
    virtual void deleteBuffer( Buffer ) = 0;

    [[nodiscard]] virtual Texture createTexture( const TextureCreateInfo&, std::span<const uint8_t> ) = 0;
    virtual void deleteTexture( Texture ) = 0;

    virtual void render( const RenderInfo& ) = 0;
    virtual void dispatch( const DispatchInfo& ) = 0;

    virtual void setResolution( uint32_t width, uint32_t height ) = 0;

    virtual uint32_t channelCount( Texture ) const = 0;

    struct CreateInfo{
        SDL_Window* window = nullptr;
        VSync vsync = {};
        std::string_view gameName{};
        uint32_t versionMajor{};
        uint32_t versionMinor{};
        uint32_t versionPatch{};
    };

protected:
    friend Engine;

    using FNCreate = Renderer*(const Renderer::CreateInfo&);
    static FNCreate* create;
    static SDL_WindowFlags windowFlag;
    static Renderer* s_instance;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void present() = 0;
};
