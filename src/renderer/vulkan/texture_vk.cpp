#include "texture_vk.hpp"

#include "utils_vk.hpp"

#include <cassert>
#include <utility>

void TextureVK::destroyResources()
{
    destroy<vkDestroySampler>( m_device, m_sampler );
    destroy<vkDestroyImageView>( m_device, m_view );
    destroy<vkDestroyImage>( m_device, m_image );
    destroy<vkFreeMemory>( m_device, m_memory );
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
    std::tie( m_image, m_view, m_memory ) = createImage(
        physDevice
        , device
        , extent
        , format
        , VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        , VK_IMAGE_ASPECT_COLOR_BIT
    );
    assert( m_image );
    assert( m_view );
    assert( m_memory );

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

    [[maybe_unused]]
    const VkResult samplerOK = vkCreateSampler( m_device, &samplerInfo, nullptr, &m_sampler );
    assert( samplerOK == VK_SUCCESS );
}

TextureVK::TextureVK( TextureVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_extent, rhs.m_extent );
    std::swap( m_image, rhs.m_image );
    std::swap( m_view, rhs.m_view );
    std::swap( m_sampler, rhs.m_sampler );
}

TextureVK& TextureVK::operator = ( TextureVK&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_memory = std::exchange( rhs.m_memory, {} );
    m_extent = std::exchange( rhs.m_extent, {} );
    m_image = std::exchange( rhs.m_image, {} );
    m_view = std::exchange( rhs.m_view, {} );
    m_sampler = std::exchange( rhs.m_sampler, {} );
    return *this;
}

void TextureVK::transferFrom( VkCommandBuffer cmd, const BufferVK& buffer )
{
    transferImage( cmd, m_image, constants::undefined, constants::copyTo );
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
    transferImage( cmd, m_image, constants::copyTo, constants::fragmentRead );
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

VkDescriptorImageInfo TextureVK::imageInfo() const
{
    return {
        .sampler = m_sampler,
        .imageView = m_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}
