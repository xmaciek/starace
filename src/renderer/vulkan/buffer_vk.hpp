#pragma once

#include <vulkan/vulkan.h>

#include <cstddef>

class BufferVK {
public:
    enum struct Purpose {
        eStaging,
        eVertex,
        eUniform,
    };

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    std::size_t m_size = 0;
    Purpose m_purpose = Purpose::eStaging;

public:
    ~BufferVK() noexcept;
    BufferVK() noexcept = default;
    BufferVK( VkPhysicalDevice, VkDevice, Purpose, std::size_t ) noexcept;
    BufferVK( const BufferVK& ) = delete;
    BufferVK& operator = ( const BufferVK& ) = delete;
    BufferVK( BufferVK&& ) noexcept;
    BufferVK& operator = ( BufferVK&& ) noexcept;

    void transferFrom( const BufferVK&, VkCommandBuffer );
    void copyData( const uint8_t* );
};
