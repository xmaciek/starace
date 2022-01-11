#pragma once

#include <vulkan/vulkan.h>

class Image {
protected:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;

    VkExtent2D m_extent = {};
    VkFormat m_format = VK_FORMAT_UNDEFINED;

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

    VkImage image() const;
    VkImageView view() const;
    VkExtent2D extent() const;
};
