#pragma once

#include "device_memory.hpp"
#include "vk.hpp"

#include <cstddef>
#include <cstdint>

class Uniform {
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
    ~Uniform() noexcept;
    Uniform() noexcept = default;
    Uniform( VkPhysicalDevice, VkDevice, std::size_t size, std::size_t minAlign ) noexcept;

    Uniform( Uniform&& ) noexcept;
    Uniform& operator = ( Uniform&& ) noexcept;
    Uniform( const Uniform& ) = delete;
    Uniform& operator = ( const Uniform& ) = delete;

    [[nodiscard]]
    VkDescriptorBufferInfo copy( const void*, std::size_t ) noexcept;
    void reset();
    void transfer( VkCommandBuffer cmd );
};
