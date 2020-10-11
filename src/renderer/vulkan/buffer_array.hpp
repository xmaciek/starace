#pragma once

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <utility>
#include <vector>

class BufferArray {
    VkDevice m_device = VK_NULL_HANDLE;
    std::vector<std::pair<VkBuffer, VkDeviceMemory>> m_buffers{};
    uint32_t m_current = 0;
    uint32_t m_size = 0;

    void destroyResources();

public:
    ~BufferArray();
    BufferArray() = default;
    BufferArray( VkPhysicalDevice, VkDevice, uint32_t size, uint32_t count );
    BufferArray( BufferArray&& ) noexcept;
    BufferArray& operator = ( BufferArray&& ) noexcept;

    VkBuffer next();
    void reset();
    uint32_t size() const;
};
