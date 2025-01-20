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

class RendererVK : public Renderer {
    SDL_Window* m_window = nullptr;
    Instance m_instance{};
    [[no_unique_address]] DebugMsg m_debugMsg{};
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    Device m_device{};

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    using Bottleneck = std::scoped_lock<std::mutex>;
    QueueManager m_queueManager{};
    CommandPool m_transferCommandPool{};
    VkCommandBuffer m_transferCmd{};

    Swapchain m_swapchain{};

    VkSemaphore m_semaphoreAvailableImage = VK_NULL_HANDLE;
    VkSemaphore m_semaphoreRender = VK_NULL_HANDLE;

    float m_lastLineWidth = 0.0f;
    uint32_t m_currentFrame = 0;
    uint32_t currentFrame();

    RenderPass m_depthPrepass{};
    RenderPass m_mainPass{};
    std::pmr::vector<Frame> m_frames{};

    static constexpr uint32_t MAX_PIPELINES = 32;
    Indexer<MAX_PIPELINES> m_pipelineIndexer{};
    std::array<uint64_t, MAX_PIPELINES> m_pipelineDescriptorIds{};
    std::array<PipelineVK, MAX_PIPELINES> m_pipelines{};
    PipelineVK* m_lastPipeline = nullptr;

    Texture m_defaultTextureId{};
    const TextureVK* m_defaultTexture = nullptr;
    Indexer<64> m_textureIndexer{};
    std::array<std::atomic<TextureVK*>, 64> m_textureSlots{};
    std::mutex m_textureBottleneck{};
    std::pmr::vector<TextureVK*> m_texturePendingDelete{};

    Indexer<64> m_bufferIndexer{};
    std::array<std::atomic<BufferVK*>, 64> m_bufferSlots{};
    std::mutex m_bufferBottleneck{};
    std::pmr::vector<BufferVK*> m_bufferPendingDelete{};

    VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

    std::atomic<uint64_t> m_pendingResolutionChange = {};
    std::optional<VSync> m_pendingVSyncChange{};

    void recreateSwapchain();
    void recreateRenderTargets( const VkExtent2D& );

public:
    virtual ~RendererVK() override;
    RendererVK( const Renderer::CreateInfo& );
    virtual bool featureAvailable( Feature ) const override;
    virtual void setVSync( VSync ) override;

    virtual PipelineSlot createPipeline( const PipelineCreateInfo& ) override;
    virtual Buffer createBuffer( std::span<const float> ) override;
    virtual Texture createTexture( const TextureCreateInfo&, std::span<const uint8_t> ) override;
    virtual uint32_t channelCount( Texture ) const override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual void deleteBuffer( Buffer ) override;
    virtual void deleteTexture( Texture ) override;
    virtual void present() override;
    virtual void push( const PushBuffer& buffer, const void* constant ) override;
    virtual void dispatch( const DispatchInfo&, const void* constant ) override;
    virtual void setResolution( uint32_t width, uint32_t height ) override;
};
