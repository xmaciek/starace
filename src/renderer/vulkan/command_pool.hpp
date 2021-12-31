#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <vector>

class CommandPool {
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_pool = VK_NULL_HANDLE;
    std::pmr::vector<VkCommandBuffer> m_buffers;

    void destroyResources() noexcept;

public:
    ~CommandPool() noexcept;
    CommandPool() noexcept = default;
    CommandPool( VkDevice device, uint32_t count, uint32_t queueFamily ) noexcept;

    CommandPool( const CommandPool& ) = delete;
    CommandPool& operator = ( const CommandPool& ) = delete;

    CommandPool( CommandPool&& ) noexcept;
    CommandPool& operator = ( CommandPool&& ) noexcept;

    VkCommandBuffer operator [] ( uint32_t );
};
