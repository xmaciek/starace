#pragma once

#include <cstdint>
#include <memory_resource>
#include <map>
#include <tuple>
#include <vector>
#include <vulkan/vulkan.h>

class BufferTransfer {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDeviceMemory m_stagingMemory = VK_NULL_HANDLE;
    VkBuffer m_staging = VK_NULL_HANDLE;
    VkBuffer m_deviceLocal = VK_NULL_HANDLE;
    size_t m_size = 0;

public:
    BufferTransfer( VkDevice, VkDeviceMemory, VkBuffer s, VkBuffer l, size_t size ) noexcept;
    BufferTransfer() noexcept = default;
    BufferTransfer( const BufferTransfer& ) noexcept = default;
    BufferTransfer& operator = ( const BufferTransfer& ) noexcept = default;
    BufferTransfer( BufferTransfer&& ) noexcept = default;
    BufferTransfer& operator = ( BufferTransfer&& ) noexcept = default;

    VkBuffer dst() const;
    VkBuffer staging() const;
    void copyToStaging( const uint8_t* );
    size_t sizeInBytes() const;
};

class BufferPool {
    VkPhysicalDevice m_physDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    std::pmr::vector<VkDeviceMemory> m_deviceMemory{};
    std::pmr::map<size_t, std::pmr::vector<BufferTransfer>> m_pending{};
    std::pmr::map<size_t, std::pmr::vector<BufferTransfer>> m_available{};

    void destroyResources();
    std::tuple<VkBuffer, VkBuffer, VkDeviceMemory, VkDeviceMemory> create( size_t );

public:
    ~BufferPool() noexcept;
    BufferPool() noexcept = default;
    BufferPool( VkPhysicalDevice, VkDevice ) noexcept;

    BufferPool( BufferPool&& ) noexcept;
    BufferPool& operator = ( BufferPool&& ) noexcept;
    BufferPool( const BufferPool& ) = delete;
    BufferPool& operator = ( const BufferPool& ) = delete;

    void reserve( size_t size, size_t count );
    BufferTransfer getBuffer( size_t sizeInBytes );
    void reset();

};
