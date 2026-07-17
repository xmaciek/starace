#pragma once

#include "vk.hpp"

#include <cstdint>
#include <variant>

class DeviceMemory {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;

public:
    ~DeviceMemory() noexcept;
    DeviceMemory() noexcept = default;
    DeviceMemory( VkPhysicalDevice physDevice, VkDevice, std::variant<VkBuffer, VkImage>, VkMemoryPropertyFlags ) noexcept;

    DeviceMemory( const DeviceMemory& ) = delete;
    DeviceMemory& operator = ( const DeviceMemory& ) = delete;

    DeviceMemory( DeviceMemory&& ) noexcept;
    DeviceMemory& operator = ( DeviceMemory&& ) noexcept;

    operator VkDeviceMemory () const noexcept;

    uint32_t size() const noexcept;
};
