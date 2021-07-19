#include "utils_vk.hpp"

#include <cassert>

static uint32_t memType( VkPhysicalDevice device, uint32_t typeBits, VkMemoryPropertyFlags flags )
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties( device, &memProperties );

    for ( uint32_t i = 0; i < memProperties.memoryTypeCount; ++i ) {
        if ( ( typeBits & ( 1 << i ) ) == 0 ) {
            continue;
        }
        if ( ( memProperties.memoryTypes[ i ].propertyFlags & flags ) != flags ) {
            continue;
        }
        return i;
    }
    assert( !"failed to find memory type" );
    return 0;
}

std::pair<VkImage, VkDeviceMemory> createImage(
    VkPhysicalDevice physDevice,
    VkDevice device,
    VkExtent2D extent,
    VkFormat format,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags
)
{
    assert( device );

    const VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {
            .width = extent.width,
            .height = extent.height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImage image = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult imageOK = vkCreateImage( device, &imageInfo, nullptr, &image );
    assert( imageOK == VK_SUCCESS );

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements( device, image, &memRequirements );

    const VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memType( physDevice, memRequirements.memoryTypeBits, memoryFlags ),
    };

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult allocOK = vkAllocateMemory( device, &allocInfo, nullptr, &imageMemory );
    assert( allocOK == VK_SUCCESS );

    [[maybe_unused]]
    const VkResult bindOK = vkBindImageMemory( device, image, imageMemory, 0 );
    assert( bindOK == VK_SUCCESS );

    return { image, imageMemory };
}

VkImageView createImageView( VkDevice device, VkImage image, VkFormat format, VkImageAspectFlagBits flags )
{
    assert( device );
    assert( image );
    static constexpr VkComponentMapping components{
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
    };
    const VkImageSubresourceRange subresourceRange{
        .aspectMask = flags,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    const VkImageViewCreateInfo imageViewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = components,
        .subresourceRange = subresourceRange,
    };

    VkImageView view = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult createOK = vkCreateImageView( device, &imageViewInfo, nullptr, &view );
    assert( createOK == VK_SUCCESS );
    assert( view != VK_NULL_HANDLE );
    return view;
}


VkFormat pickSupportedFormat( VkPhysicalDevice physicalDevice, const std::pmr::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags flags )
{
    for ( VkFormat it : formats ) {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties( physicalDevice, it, &props );
        switch ( tiling ) {
        case VK_IMAGE_TILING_LINEAR:
            if ( ( props.linearTilingFeatures & flags ) == flags ) {
                return it;
            }
            continue;
        case VK_IMAGE_TILING_OPTIMAL:
            if ( ( props.optimalTilingFeatures & flags ) == flags ) {
                return it;
            }
            continue;
        default:
            assert( !"unsupported tiling" );
        }
    }
    assert( !"format not found" );
    return {};
}


uint32_t memoryType( VkPhysicalDevice device, uint32_t typeBits, VkMemoryPropertyFlags flags )
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties( device, &memProperties );

    for ( uint32_t i = 0; i < memProperties.memoryTypeCount; ++i ) {
        if ( ( typeBits & ( 1 << i ) ) == 0 ) {
            continue;
        }
        if ( ( memProperties.memoryTypes[ i ].propertyFlags & flags ) != flags ) {
            continue;
        }
        return i;
    }
    assert( !"failed to find requested memory type" );
    return 0;
}
