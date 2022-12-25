#pragma once

#include "device_memory.hpp"
#include "utils_vk.hpp"
#include "vk.hpp"

class Image {
protected:
    DeviceMemory m_deviceMemory{};
    VkDevice m_device = VK_NULL_HANDLE;

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;

    VkExtent2D m_extent = {};
    VkFormat m_format = VK_FORMAT_UNDEFINED;

    TransferInfo m_currentLocation = constants::undefined;
    uint32_t m_mipCount = 0;

public:
    ~Image() noexcept;
    Image() noexcept = default;
    Image( VkPhysicalDevice
        , VkDevice
        , VkExtent2D
        , VkFormat
        , uint32_t mipCount
        , VkImageUsageFlags
        , VkMemoryPropertyFlags
        , VkImageAspectFlagBits
    ) noexcept;

    Image( Image&& ) noexcept;
    Image& operator = ( Image&& ) noexcept;

    Image( const Image& ) = delete;
    Image& operator = ( const Image& ) = delete;

    uint32_t mipCount() const;
    VkImage image() const;
    VkImageView view() const;
    VkExtent2D extent() const;

    void transfer( VkCommandBuffer, const TransferInfo& );
};
