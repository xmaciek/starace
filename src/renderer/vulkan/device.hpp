#pragma once

#include "vk.hpp"

#include <span>

class Device {
private:
    VkDevice m_device{};

public:
    ~Device();
    Device() = default;
    Device( VkPhysicalDevice, std::span<const char*> layers, std::span<const VkDeviceQueueCreateInfo> queues );
    Device( const Device& ) = delete;
    Device& operator = ( const Device& ) = delete;
    Device( Device&& );
    Device& operator = ( Device&& );

    inline operator VkDevice () const { return m_device; }
};
