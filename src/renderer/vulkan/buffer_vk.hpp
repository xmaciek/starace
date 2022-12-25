#pragma once

#include "device_memory.hpp"
#include "vk.hpp"

class BufferVK {
public:
    struct Purpose {
        VkBufferUsageFlags usage{};
        VkMemoryPropertyFlags flags{};
    };
    static constexpr Purpose STAGING{
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    static constexpr Purpose DEVICE_LOCAL{
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    };

private:
    DeviceMemory m_memory{};
    VkDevice m_device = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    uint32_t m_size = 0;

public:
    ~BufferVK() noexcept;
    BufferVK() noexcept = default;
    BufferVK( VkPhysicalDevice, VkDevice, const Purpose&, uint32_t ) noexcept;
    BufferVK( const BufferVK& ) = delete;
    BufferVK& operator = ( const BufferVK& ) = delete;
    BufferVK( BufferVK&& ) noexcept;
    BufferVK& operator = ( BufferVK&& ) noexcept;

    void transferFrom( const BufferVK&, VkCommandBuffer );
    void copyData( const uint8_t* );
    uint32_t sizeInBytes() const;

    operator VkBuffer () const;
};
