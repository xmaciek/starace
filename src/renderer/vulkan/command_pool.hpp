#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <vector>

class CommandPool {
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_pool = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;
    std::pmr::vector<VkCommandBuffer> m_buffers;
    uint32_t m_queueIndex = 0;
    uint32_t m_currentFrame = 0;

    void destroyResources() noexcept;

public:
    ~CommandPool() noexcept;
    CommandPool() noexcept = default;
    CommandPool( VkDevice device, uint32_t commandBuffersCount, uint32_t queueFamily, uint32_t queueIdx ) noexcept;

    CommandPool( const CommandPool& ) = delete;
    CommandPool& operator = ( const CommandPool& ) = delete;

    CommandPool( CommandPool&& ) noexcept;
    CommandPool& operator = ( CommandPool&& ) noexcept;

    VkCommandPool pool() const noexcept;
    VkCommandBuffer buffer() const noexcept;
    VkQueue queue() const noexcept;
    uint32_t queueIndex() const noexcept;

    void transferBufferAndWait( VkBuffer src, VkBuffer dst, size_t size ) const;

    void setFrame( uint32_t ) noexcept;
};
