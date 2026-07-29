#pragma once

#include "buffer_vk.hpp"
#include "command_pool.hpp"
#include "debug_messanger.hpp"
#include "device.hpp"
#include "frame.hpp"
#include "instance.hpp"
#include "pipeline_vk.hpp"
#include "queue_manager.hpp"
#include "renderpass.hpp"
#include "swapchain.hpp"
#include "texture_vk.hpp"
#include "uniform.hpp"
#include "vk.hpp"

#include <renderer/renderer.hpp>
#include <shared/indexer.hpp>

#include <array>
#include <memory_resource>
#include <vector>
#include <mutex>
#include <optional>
#include <variant>

class Surface {
    VkSurfaceKHR m_surface{};
    VkInstance m_instance{};
public:
    ~Surface();
    Surface() = default;
    Surface( const Surface& ) = delete;
    Surface& operator = ( const Surface& ) = delete;
    Surface( Surface&& );
    Surface& operator = ( Surface&& );
    Surface( SDL_Window*, VkInstance );

    inline operator VkSurfaceKHR () const { return m_surface; }
};

class Semaphore {
    VkSemaphore m_semaphore{};
    VkDevice m_device{};

public:
    ~Semaphore();
    Semaphore() = default;
    Semaphore( const Semaphore& ) = delete;
    Semaphore& operator = ( const Semaphore& ) = delete;
    Semaphore( Semaphore&& );
    Semaphore& operator = ( Semaphore&& );
    Semaphore( VkDevice );

    inline operator VkSemaphore () const { return m_semaphore; }
    inline operator VkSemaphore* () { return &m_semaphore; }
};

class RendererVK : public Renderer {
public:
    enum : uint32_t {
        MAX_BUFFERS = 32,
        MAX_PIPELINES = 32,
        MAX_TEXTURES = 64,
        STAGING_BUFFER_CACHE_COUNT = 4,
    };
    SDL_Window* m_window = nullptr;
    Instance m_instance{};
    [[no_unique_address]] DebugMsg m_debugMsg{};
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    Device m_device{};

    Surface m_surface{};

    using Bottleneck = std::scoped_lock<std::mutex>;
    QueueManager m_queueManager{};
    CommandPool m_transferCommandPool{};
    VkCommandBuffer m_transferCmd{};

    Swapchain m_swapchain{};

    Semaphore m_semaphoreAvailableImage{};
    Semaphore m_semaphoreRender{};

    float m_lastLineWidth = 0.0f;
    uint32_t m_currentFrame = 0;
    uint32_t currentFrame();

    std::pmr::vector<Frame> m_frames{};

    Indexer<MAX_PIPELINES> m_pipelineIndexer{};
    std::array<uint64_t, MAX_PIPELINES> m_pipelineDescriptorIds{};
    std::array<PipelineVK, MAX_PIPELINES> m_pipelines{};
    PipelineVK* m_lastPipeline = nullptr;

    Texture m_defaultTextureId{};
    const TextureVK* m_defaultTexture = nullptr;
    Indexer<MAX_TEXTURES> m_textureIndexer{};
    std::array<std::atomic<TextureVK*>, MAX_TEXTURES> m_textureSlots{};

    Indexer<MAX_BUFFERS> m_bufferIndexer{};
    std::array<std::atomic<BufferVK*>, MAX_BUFFERS> m_bufferSlots{};

    std::mutex m_resourceDeleteBottleneck{};
    using ResourceDelete = std::variant<TextureVK*, BufferVK*>;
    std::pmr::vector<ResourceDelete> m_resourceDelete{};

    std::mutex m_stagingBuffersCacheBottleneck{};
    std::array<BufferVK, STAGING_BUFFER_CACHE_COUNT> m_stagingBuffersCache{};

    VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_resolution{};

    std::atomic<uint64_t> m_pendingResolutionChange = {};
    std::optional<VSync> m_pendingVSyncChange{};

    void recreateSwapchain();
    void recreateRenderTargets( VkExtent2D );
    void refreshResolution();

    BufferVK getStagingBuffer( uint32_t );
    void releaseStagingBuffer( BufferVK&& );

public:
    virtual ~RendererVK() override;
    RendererVK( const Renderer::CreateInfo& );
    virtual bool featureAvailable( Feature ) const override;
    virtual void setFeatureEnabled( Feature, bool ) override;
    virtual void setVSync( VSync ) override;

    virtual PipelineSlot createPipeline( const PipelineCreateInfo& ) override;
    virtual Buffer createBuffer( std::span<const uint8_t> ) override;
    virtual Texture createTexture( const TextureCreateInfo&, std::span<const uint8_t> ) override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual void deleteBuffer( Buffer ) override;
    virtual void deleteTexture( Texture ) override;
    virtual void present() override;
    virtual void render( const RenderInfo& ) override;
    virtual void dispatch( const DispatchInfo& ) override;
    virtual void setResolution( uint32_t width, uint32_t height ) override;
    virtual void pushScissor( math::vec4 rect, math::vec2 viewportExtent ) override;
    virtual void popScissor() override;
};
