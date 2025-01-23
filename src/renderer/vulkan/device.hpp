#pragma once

#include "vk.hpp"

#include <bitset>
#include <cstdint>
#include <span>

class Device {
public:
    enum Feature : uint32_t {
        eVRS,
        count,
    };

private:
    VkDevice m_device{};
    std::bitset<count> m_features{};

public:
    ~Device();
    Device() = default;
    Device( VkPhysicalDevice, std::span<const char*> layers, std::span<const VkDeviceQueueCreateInfo> queues );
    Device( const Device& ) = delete;
    Device& operator = ( const Device& ) = delete;
    Device( Device&& );
    Device& operator = ( Device&& );

    inline operator VkDevice () const { return m_device; }

    bool hasFeature( Feature ) const;
};
