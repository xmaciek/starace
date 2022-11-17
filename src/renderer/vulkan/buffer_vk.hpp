#pragma once

#include "device_memory.hpp"

#include <vulkan/vulkan.h>

class BufferVK {
public:
    enum struct Purpose {
        eStaging,
        eVertex,
    };

private:
    DeviceMemory m_memory{};
    VkDevice m_device = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    uint32_t m_size = 0;
    Purpose m_purpose = Purpose::eStaging;

public:
    ~BufferVK() noexcept;
    BufferVK() noexcept = default;
    BufferVK( VkPhysicalDevice, VkDevice, Purpose, uint32_t ) noexcept;
    BufferVK( const BufferVK& ) = delete;
    BufferVK& operator = ( const BufferVK& ) = delete;
    BufferVK( BufferVK&& ) noexcept;
    BufferVK& operator = ( BufferVK&& ) noexcept;

    void transferFrom( const BufferVK&, VkCommandBuffer );
    void copyData( const uint8_t* );
    uint32_t sizeInBytes() const;

    operator VkBuffer () const;
};
