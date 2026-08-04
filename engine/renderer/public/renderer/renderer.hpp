#pragma once

#include <renderer/buffer.hpp>
#include <renderer/display_mode.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>
#include <shared/stack_vector.hpp>
#include <math.hpp>

#include <SDL.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory_resource>
#include <vector>
#include <span>
#include <string_view>

struct Uniform {
    const void* ptr{};
    size_t size{};
    inline Uniform() = default;
    inline Uniform( const Uniform& ) = default;
    inline Uniform& operator = ( const Uniform& ) = default;

    template <typename T>
    inline Uniform( const T& u ) requires std::is_trivially_copyable_v<T> : ptr{ &u }, size{ sizeof( u ) } {}

    template <typename T>
    inline Uniform( const std::pmr::vector<T>& u ) requires std::is_trivially_copyable_v<T>
    : ptr{ u.data() }, size{ u.size() * sizeof( T ) } {}

    template <typename T>
    inline Uniform( std::span<const T>& u ) requires std::is_trivially_copyable_v<T>
    : ptr{ u.data() }, size{ u.size() * sizeof( T ) } {}
};

struct DispatchInfo {
    PipelineSlot m_pipeline = 0;
    Uniform m_uniform{};
};

struct RenderInfo {
    enum : uint32_t {
        MAX_TEXTURES = 9,
        MAX_STORAGES = 4,
    };
    PipelineSlot m_pipeline{};
    uint32_t m_verticeCount = 0;
    uint32_t m_instanceCount = 1;
    float m_lineWidth = 1.0f;
    Buffer m_vertexBuffer{};
    Buffer m_indexBuffer{};
    Uniform m_uniform{};
    StackVector<Texture, MAX_TEXTURES> m_fragmentTexture{};
    StackVector<Buffer, MAX_STORAGES> m_storageBuffers{};
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
    virtual void pushScissor( math::vec4 rect, math::vec2 viewportExtent ) = 0;
    virtual void popScissor() = 0;

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


template <typename TPushConstant>
struct InstancedRendering {
    using Instance = typename TPushConstant::Instance;
    Renderer* renderer{};
    RenderInfo renderInfo{ .m_instanceCount = 0, };
    TPushConstant pushConstant{};

    InstancedRendering( Renderer* r, PipelineSlot p )
    : renderer{ r }
    {
        renderInfo.m_pipeline = p;
        renderInfo.m_verticeCount = pushConstant.VERTICES;
        renderInfo.m_uniform = pushConstant;
    }

    ~InstancedRendering()
    {
        flush();
    }

    inline void flush()
    {
        assert( renderer );
        assert( renderInfo.m_instanceCount <= pushConstant.INSTANCES );
        if ( renderInfo.m_instanceCount == 0 ) [[unlikely]] return;
        renderer->render( renderInfo );
        renderInfo.m_instanceCount = 0;
    }

    inline void append( const Instance& i )
    {
        assert( renderer );
        assert( renderInfo.m_instanceCount < pushConstant.INSTANCES );
        pushConstant.m_instances[ renderInfo.m_instanceCount++ ] = i;
        if ( renderInfo.m_instanceCount < pushConstant.INSTANCES ) [[likely]] return;
        flush();
    }
};
