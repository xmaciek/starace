#include "image.hpp"

#include "utils_vk.hpp"

#include <profiler.hpp>

#include <cassert>
#include <utility>

Image::~Image() noexcept
{
    destroy<vkDestroyImageView>( m_device, m_imageView );
    destroy<vkDestroyImage>( m_device, m_image );
}

Image::Image( Image&& img ) noexcept
{
    std::swap( m_device, img.m_device );
    std::swap( m_deviceMemory, img.m_deviceMemory );
    std::swap( m_image, img.m_image );
    std::swap( m_imageView, img.m_imageView );
    std::swap( m_extent, img.m_extent );
    std::swap( m_format, img.m_format );
    std::swap( m_currentLocation, img.m_currentLocation );
    std::swap( m_mipCount, img.m_mipCount );
    std::swap( m_arrayCount, img.m_arrayCount );
}

Image& Image::operator = ( Image&& img ) noexcept
{
    std::swap( m_device, img.m_device );
    std::swap( m_deviceMemory, img.m_deviceMemory );
    std::swap( m_image, img.m_image );
    std::swap( m_imageView, img.m_imageView );
    std::swap( m_extent, img.m_extent );
    std::swap( m_format, img.m_format );
    std::swap( m_currentLocation, img.m_currentLocation );
    std::swap( m_mipCount, img.m_mipCount );
    std::swap( m_arrayCount, img.m_arrayCount );
    return *this;
}

VkImage Image::image() const
{
    assert( m_image );
    return m_image;
}

VkImageView Image::view() const
{
    assert( m_imageView );
    return m_imageView;
}

VkExtent2D Image::extent() const
{
    return m_extent;
}

uint32_t Image::mipCount() const
{
    assert( m_mipCount > 0 );
    return m_mipCount;
}

uint32_t Image::arrayCount() const
{
    assert( m_arrayCount > 0 );
    return m_arrayCount;
}

Image::Image( VkPhysicalDevice physDevice
    , VkDevice device
    , VkExtent2D extent
    , VkFormat format
    , uint32_t mipCount
    , uint32_t arrayCount
    , VkImageUsageFlags usageFlags
    , VkMemoryPropertyFlags memoryFlags
    , VkImageAspectFlagBits aspectFlags
) noexcept
: m_device{ device }
, m_extent{ extent }
, m_format{ format }
, m_mipCount{ mipCount }
, m_arrayCount{ arrayCount }
{
    ZoneScoped;

    assert( mipCount > 0 );
    assert( arrayCount > 0 );
    const VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {
            .width = extent.width,
            .height = extent.height,
            .depth = 1,
        },
        .mipLevels = mipCount,
        .arrayLayers = arrayCount,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    [[maybe_unused]]
    const VkResult imageOK = vkCreateImage( m_device, &imageInfo, nullptr, &m_image );
    assert( imageOK == VK_SUCCESS );

    m_deviceMemory = DeviceMemory{ physDevice, m_device, m_image, memoryFlags };

    [[maybe_unused]]
    const VkResult bindOK = vkBindImageMemory( m_device, m_image, m_deviceMemory, 0 );
    assert( bindOK == VK_SUCCESS );

    static constexpr VkComponentMapping components{
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    const VkImageSubresourceRange subresourceRange{
        .aspectMask = static_cast<VkImageAspectFlags>( aspectFlags ),
        .baseMipLevel = 0,
        .levelCount = mipCount,
        .baseArrayLayer = 0,
        .layerCount = arrayCount == 1 ? 1 : VK_REMAINING_ARRAY_LAYERS,
    };

    const VkImageViewCreateInfo imageViewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = arrayCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        .format = m_format,
        .components = components,
        .subresourceRange = subresourceRange,
    };

    [[maybe_unused]]
    const VkResult createOK = vkCreateImageView( m_device, &imageViewInfo, nullptr, &m_imageView );
    assert( createOK == VK_SUCCESS );
}

void Image::transfer( VkCommandBuffer cmd, const TransferInfo& dst )
{
    if ( m_currentLocation == dst ) {
        return;
    }
    transferImage( cmd, m_image, m_currentLocation, dst, m_mipCount, m_arrayCount );
    m_currentLocation = dst;
}
