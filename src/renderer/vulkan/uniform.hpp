#pragma once

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>

class Uniform {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDeviceMemory m_memoryStaging = VK_NULL_HANDLE;
    VkDeviceMemory m_memoryDeviceLocal = VK_NULL_HANDLE;
    VkBuffer m_staging = VK_NULL_HANDLE;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    void* m_mapped = nullptr;
    std::uintptr_t m_currentOffset = 0;
    std::size_t m_minAlign = 0;
    std::size_t m_transferSize = 0;
    std::size_t m_size = 0;

    void destroyResources() noexcept;

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