#pragma once

#include <renderer/renderer.hpp>

#include "buffer_vk.hpp"
#include "renderpass.hpp"
#include "command_pool.hpp"
#include "pipeline_vk.hpp"
#include "swapchain.hpp"
#include "texture_vk.hpp"
#include "buffer_pool.hpp"
#include "debug_messanger.hpp"
#include "render_target.hpp"

#include <vulkan/vulkan.h>

#include <vector>
#include <deque>
#include <map>

class RendererVK : public Renderer {
    SDL_Window* m_window = nullptr;

    DebugMsg m_debugMsg{};
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    uint32_t m_queueFamilyGraphics = 0;
    uint32_t m_queueFamilyPresent = 0;
    uint32_t m_queueFamilyTransfer = 0;
    VkQueue m_queuePresent{};

    CommandPool m_graphicsCmd{};
    CommandPool m_transferCmd{};
    CommandPool m_transferToGraphicsCmd{};

    VkDevice m_device = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    Swapchain m_swapchain{};

    std::pmr::vector<RenderTarget> m_mainTargets;

    VkSemaphore m_semaphoreAvailableImage = VK_NULL_HANDLE;
    VkSemaphore m_semaphoreRender = VK_NULL_HANDLE;

    uint32_t m_currentFrame = 0;
    uint32_t currentFrame();

    BufferPool m_uniforms[ 3 ];
    std::pmr::vector<BufferTransfer> m_pending{};

    RenderPass m_mainPass{};
    std::array<PipelineVK, (size_t)Pipeline::count> m_pipelines{};
    PipelineVK* m_lastPipeline = nullptr;

    std::pmr::vector<TextureVK*> m_textures;

    std::pmr::map<Buffer, BufferVK> m_bufferMap{};

    VkFormat m_depthFormat = {};
    
    void transferBufferAndWait( VkBuffer src, VkBuffer dst, size_t size );

    void flushUniforms();

public:
    virtual ~RendererVK() override;
    RendererVK( SDL_Window* );

    virtual Buffer createBuffer( std::pmr::vector<float>&&, Buffer::Lifetime ) override;
    virtual std::pmr::memory_resource* allocator() override;
    virtual Texture createTexture( uint32_t w, uint32_t h, Texture::Format, bool, const uint8_t* ) override;
    virtual void beginFrame() override;
    virtual void clear() override;
    virtual void deleteBuffer( const Buffer& ) override;
    virtual void deleteTexture( Texture ) override;
    virtual void present() override;
    virtual void push( void* buffer, void* constant ) override;
    virtual void setViewportSize( uint32_t w, uint32_t h ) override;
    virtual void submit() override;
};
