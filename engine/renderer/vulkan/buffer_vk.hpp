#pragma once

#include "device_memory.hpp"
#include "vk.hpp"

#include <shared/stack_vector.hpp>

#include <cstdint>
#include <span>

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
    StackVector<VkDescriptorBufferInfo, 16> m_subAllocations{};

public:
    ~BufferVK() noexcept;
    BufferVK() noexcept = default;
    BufferVK( VkPhysicalDevice, VkDevice, const Purpose&, VkDeviceSize size ) noexcept;
    BufferVK( const BufferVK& ) = delete;
    BufferVK& operator = ( const BufferVK& ) = delete;
    BufferVK( BufferVK&& ) noexcept;
    BufferVK& operator = ( BufferVK&& ) noexcept;

    void transferFrom( const BufferVK&, VkCommandBuffer );
    void copyData( std::span<const uint8_t> );
    void copyDatas( std::span<std::span<const uint8_t>>, VkDeviceSize minAlignment );

    VkDeviceSize sizeInBytes() const;
    VkDeviceSize subAllocationSize() const;
    static VkDeviceSize computeSize( std::span<std::span<const uint8_t>> datas, VkDeviceSize minAlignment );

    VkDescriptorBufferInfo bindInfo() const;
    std::span<const VkDescriptorBufferInfo> subAllocations() const;
    [[nodiscard]]
    bool empty() const;

    operator VkBuffer () const;
    void release( const VkDescriptorBufferInfo& );

    void bind( VkCommandBuffer );
    void bindAsIndex( VkCommandBuffer );
};
