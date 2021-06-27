#pragma once

#include <vulkan/vulkan.h>
#include <memory_resource>
#include <vector>

class CommandPool {
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_pool = VK_NULL_HANDLE;
    std::pmr::vector<VkQueue> m_queues;
    std::pmr::vector<VkCommandBuffer> m_buffers;
    uint32_t m_currentFrame = 0;

    void destroy() noexcept;

public:
    ~CommandPool() noexcept;
    CommandPool() noexcept = default;
    CommandPool( VkDevice device, uint32_t commandBuffersCount, std::pair<uint32_t, uint32_t> queueCount, uint32_t queueFamily ) noexcept;

    CommandPool( const CommandPool& ) = delete;
    CommandPool& operator = ( const CommandPool& ) = delete;

    CommandPool( CommandPool&& ) noexcept;
    CommandPool& operator = ( CommandPool&& ) noexcept;

    VkCommandPool pool() const noexcept;
    VkCommandBuffer buffer() const noexcept;
    VkQueue queue() const noexcept;

    void transferBufferAndWait( VkBuffer src, VkBuffer dst, size_t size ) const;

    void setFrame( uint32_t ) noexcept;
};
