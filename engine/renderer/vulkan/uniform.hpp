#pragma once

#include "device_memory.hpp"
#include "vk.hpp"

#include <cstddef>
#include <cstdint>

class UniformVK {
    DeviceMemory m_memoryStaging{};
    DeviceMemory m_memoryDeviceLocal{};
    VkDevice m_device = VK_NULL_HANDLE;
    VkBuffer m_staging = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    void* m_mapped = nullptr;
    std::uintptr_t m_currentOffset = 0;
    std::size_t m_minAlign = 0;
    std::size_t m_size = 0;

public:
    ~UniformVK() noexcept;
    UniformVK() noexcept = default;
    UniformVK( VkPhysicalDevice, VkDevice, std::size_t size, std::size_t minAlign ) noexcept;

    UniformVK( UniformVK&& ) noexcept;
    UniformVK& operator = ( UniformVK&& ) noexcept;
    UniformVK( const UniformVK& ) = delete;
    UniformVK& operator = ( const UniformVK& ) = delete;

    [[nodiscard]]
    VkDescriptorBufferInfo copy( const void*, std::size_t size, std::size_t sizeHint ) noexcept;
    void reset();
    void transfer( VkCommandBuffer cmd );
};
