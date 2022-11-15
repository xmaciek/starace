#pragma once

#include <renderer/renderer.hpp>

#include "buffer_vk.hpp"
#include "renderpass.hpp"
#include "command_pool.hpp"
#include "pipeline_vk.hpp"
#include "swapchain.hpp"
#include "texture_vk.hpp"
#include "debug_messanger.hpp"
#include "render_target.hpp"
#include "uniform.hpp"
#include "frame.hpp"

#include <shared/indexer.hpp>

#include <vulkan/vulkan.h>

#include <array>
#include <memory_resource>
#include <vector>
#include <mutex>
#include <optional>

class RendererVK : public Renderer {
    SDL_Window* m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    [[no_unique_address]]
    DebugMsg m_debugMsg{};

    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    uint32_t m_queueFamilyGraphics = 0;
    uint32_t m_queueFamilyPresent = 0;
    VkQueue m_queuePresent{};

    using Bottleneck = std::scoped_lock<std::mutex>;
    std::array<std::mutex, 2> m_cmdBottleneck{};
    std::tuple<VkQueue, std::mutex*> m_queueGraphics{};
    std::tuple<VkQueue, std::mutex*> m_queueTransfer{};

    CommandPool m_commandPool{};
    Swapchain m_swapchain{};

    VkSemaphore m_semaphoreAvailableImage = VK_NULL_HANDLE;
    VkSemaphore m_semaphoreRender = VK_NULL_HANDLE;

    float m_lastLineWidth = 0.0f;
    uint32_t m_currentFrame = 0;
    uint32_t currentFrame();

    RenderPass m_depthPrepass{};
    RenderPass m_mainPass{};
    std::pmr::vector<Frame> m_frames{};

    std::array<Bindpoints, 32> m_pipelineDescriptorIds{};
    std::array<PipelineVK, 32> m_pipelines{};
    PipelineVK* m_lastPipeline = nullptr;

    Indexer<64> m_textureIndexer{};
    std::array<std::atomic<TextureVK*>, 64> m_textureSlots{};
    std::mutex m_textureBottleneck{};
    std::pmr::vector<TextureVK*> m_texturePendingDelete{};

    Indexer<64> m_bufferIndexer{};
    std::array<std::atomic<BufferVK*>, 64> m_bufferSlots{};
    std::mutex m_bufferBottleneck{};
    std::pmr::vector<BufferVK*> m_bufferPendingDelete{};

    VkFormat m_depthFormat = {};

    std::optional<VSync> m_pendingVSyncChange{};
    [[nodiscard]]
    VkCommandBuffer flushUniforms();
    void recreateSwapchain();
    void recreateRenderTargets();

public:
    virtual ~RendererVK() override;
    RendererVK( SDL_Window*, VSync );
    virtual void setVSync( VSync ) override;

    virtual void createPipeline( const PipelineCreateInfo& ) override;
    virtual Buffer createBuffer( std::pmr::vector<float>&& ) override;
    virtual Buffer createBuffer( std::span<const float> ) override;
    virtual std::pmr::memory_resource* allocator() override;
    virtual Texture createTexture( const TextureCreateInfo&, std::pmr::vector<uint8_t>&& ) override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual void deleteBuffer( Buffer ) override;
    virtual void deleteTexture( Texture ) override;
    virtual void present() override;
    virtual void push( const PushBuffer& buffer, const void* constant ) override;
    virtual void dispatch( const DispatchInfo&, const void* constant ) override;
};
