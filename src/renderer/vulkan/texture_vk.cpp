#include "texture_vk.hpp"

#include <cassert>
#include <iostream>

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
    assert( !"failed to find requested memory type" );
    std::cout << "failedto find requested memory type" << std::endl;
    return 0;
}

void TextureVK::destroyResources()
{
    if ( m_view ) {
        vkDestroyImageView( m_device, m_view, nullptr );
    }
    if ( m_sampler ) {
        vkDestroySampler( m_device, m_sampler, nullptr );
    }
    if ( m_image ) {
        vkDestroyImage( m_device, m_image, nullptr );
    }
    if ( m_memory ) {
        vkFreeMemory( m_device, m_memory, nullptr );
    }
}

TextureVK::~TextureVK()
{
    destroyResources();
}

TextureVK::TextureVK( VkPhysicalDevice physDevice, VkDevice device, VkExtent2D extent, VkFormat format )
: m_device{ device }
, m_extent{ extent }
{
    assert( extent.width > 0 );
    assert( extent.height > 0 );
    const VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = VkExtent3D{ .width = extent.width, .height = extent.height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = m_layout,
    };
    if ( const VkResult res = vkCreateImage( m_device, &imageInfo, nullptr, &m_image );
        res != VK_SUCCESS ) {
        assert( !"failed to create image" );
        return;
    }

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements( device, m_image, &memRequirements );

    const VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memType( physDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ),
    };

    if ( const VkResult res = vkAllocateMemory(device, &allocInfo, nullptr, &m_memory );
        res != VK_SUCCESS ) {
        assert( !"failed to allocate texture image memory" );
        std::cout << "failed to allocate texture image memory" << std::endl;
        return;
    }

    if ( const VkResult res = vkBindImageMemory( device, m_image, m_memory, 0 );
        res != VK_SUCCESS ) {
        assert( !"failed to bind image to memory" );
        std::cout << "failed to bind image to memory" << std::endl;
        return;
    }

    const VkImageSubresourceRange subresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    const VkImageViewCreateInfo viewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = subresourceRange,
    };
    if ( const VkResult res = vkCreateImageView( m_device, &viewInfo, nullptr, &m_view );
        res != VK_SUCCESS ) {
        assert( !"failed to create image view" );
        std::cout << "failed to create image view" << std::endl;
        return;
    }

    const VkSamplerCreateInfo samplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .unnormalizedCoordinates = VK_FALSE,
    };
    if ( const VkResult res = vkCreateSampler( m_device, &samplerInfo, nullptr, &m_sampler );
        res != VK_SUCCESS ) {
        assert( !"failed to create sampler" );
        std::cout << "failed to create sampler" << std::endl;
        return;
    }

}

TextureVK::TextureVK( TextureVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_extent, rhs.m_extent );
    std::swap( m_image, rhs.m_image );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_view, rhs.m_view );
    std::swap( m_sampler, rhs.m_sampler );
    std::swap( m_currentAccess, rhs.m_currentAccess );
    std::swap( m_currentStage, rhs.m_currentStage );
}

TextureVK& TextureVK::operator = ( TextureVK&& rhs ) noexcept
{
    destroyResources();
    m_device = rhs.m_device;
    m_memory = rhs.m_memory;
    m_extent = rhs.m_extent;
    m_image = rhs.m_image;
    m_layout = rhs.m_layout;
    m_view = rhs.m_view;
    m_sampler = rhs.m_sampler;
    m_currentAccess = rhs.m_currentAccess;
    m_currentStage = rhs.m_currentStage;
    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_memory = VK_NULL_HANDLE;
    rhs.m_extent = {};
    rhs.m_image = VK_NULL_HANDLE;
    rhs.m_view = VK_NULL_HANDLE;
    rhs.m_sampler = VK_NULL_HANDLE;
    rhs.m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    rhs.m_currentAccess = 0;
    rhs.m_currentStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    return *this;
}

void TextureVK::transferFrom( VkCommandBuffer cmd, const BufferVK& buffer )
{
    transitionLayout( cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
    const VkBufferImageCopy region{
        .imageSubresource = VkImageSubresourceLayers{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
        },
        .imageExtent = VkExtent3D{
            .width = m_extent.width,
            .height = m_extent.height,
            .depth = 1,
        },
    };
    vkCmdCopyBufferToImage( cmd, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );
    transitionLayout( cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
}

void TextureVK::transitionLayout( VkCommandBuffer cmd, VkImageLayout dstLayout )
{
    const VkImageSubresourceRange subresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = m_currentAccess,
        .oldLayout = m_layout,
        .newLayout = dstLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_image,
        .subresourceRange = subresourceRange,
    };

    VkPipelineStageFlags dstStage{};

    switch ( m_layout ) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        if ( dstLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) {
            assert( !"unsupported transfer flag" );
            std::cout << "unsupported transfer flag" << std::endl;
            return;
        }
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        if ( dstLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ) {
            assert( !"unsupported transfer flag" );
            std::cout << "unsupported transfer flag" << std::endl;
            return;
        }
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        break;

    default:
        assert( !"invalid current image layout" );
        std::cout << "invalid current image layout" << std::endl;
        return;
    }
    vkCmdPipelineBarrier( cmd, m_currentStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier );

    m_layout = dstLayout;
    m_currentAccess = barrier.dstAccessMask;
    m_currentStage = dstStage;
}

VkImageView TextureVK::view() const
{
    assert( m_view );
    return m_view;
}

VkSampler TextureVK::sampler() const
{
    assert( m_sampler );
    return m_sampler;
}
