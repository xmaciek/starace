#pragma once

#include "device_memory.hpp"
#include "utils_vk.hpp"
#include "vk.hpp"

class Image {
public:
    struct Purpose {
        uint32_t usage = 0;
        VkMemoryPropertyFlagBits memoryFlags{};
        VkImageAspectFlagBits aspectFlags{};
    };
    static constexpr inline Purpose RTGT_COLOR{
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    };
    static constexpr inline Purpose RTGT_DEPTH{
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
    };

protected:
    DeviceMemory m_deviceMemory{};
    VkDevice m_device = VK_NULL_HANDLE;

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;

    VkExtent2D m_extent = {};
    VkFormat m_format = VK_FORMAT_UNDEFINED;

    TransferInfo m_currentLocation = constants::undefined;
    uint32_t m_mipCount = 0;
    uint32_t m_arrayCount = 0;

public:
    ~Image() noexcept;
    Image() noexcept = default;
    Image( VkPhysicalDevice
        , VkDevice
        , VkExtent2D
        , VkFormat
        , uint32_t mipCount
        , uint32_t arrayCount
        , VkImageUsageFlags
        , VkMemoryPropertyFlags
        , VkImageAspectFlagBits
    ) noexcept;

    Image( Image&& ) noexcept;
    Image& operator = ( Image&& ) noexcept;

    Image( const Image& ) = delete;
    Image& operator = ( const Image& ) = delete;

    uint32_t mipCount() const;
    uint32_t arrayCount() const;
    VkImage image() const;
    VkDescriptorImageInfo imageInfo() const;
    VkImageView view() const;
    VkExtent2D extent() const;

    void transfer( VkCommandBuffer, const TransferInfo& );
};
