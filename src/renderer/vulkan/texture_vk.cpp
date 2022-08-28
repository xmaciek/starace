#include "texture_vk.hpp"

#include "utils_vk.hpp"

#include <Tracy.hpp>

#include <cassert>
#include <utility>

static auto format( const TextureCreateInfo& tci )
{
    switch ( tci.format ) {
    case TextureFormat::eR: return VK_FORMAT_R8_UNORM;
    case TextureFormat::eRGBA: return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::eBGRA: return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::eBC1_unorm: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case TextureFormat::eBC2_unorm: return VK_FORMAT_BC2_UNORM_BLOCK;
    case TextureFormat::eBC3_unorm: return VK_FORMAT_BC3_UNORM_BLOCK;
    default:
        assert( !"unsuported format" );
        return VK_FORMAT_UNDEFINED;
    };
}

static auto addressMode( TextureAddressMode a )
{
    switch ( a ) {
    case TextureAddressMode::eClamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case TextureAddressMode::eRepeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureAddressMode::eMirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    default:
        assert( !"unsuported address mode" );
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    };
}

TextureVK::~TextureVK()
{
    destroy<vkDestroySampler>( m_device, m_sampler );
}

TextureVK::TextureVK( VkPhysicalDevice physDevice, VkDevice device, VkExtent2D extent, VkFormat format )
: Image{
    physDevice
    , device
    , extent
    , format
    , 1
    , VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    , VK_IMAGE_ASPECT_COLOR_BIT
}
{
    ZoneScoped;
    assert( extent.width > 0 );
    assert( extent.height > 0 );

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

TextureVK::TextureVK( const TextureCreateInfo& tci, VkPhysicalDevice physDevice, VkDevice device )
: Image{
    physDevice
    , device
    , { .width = tci.width, .height = tci.height }
    , format( tci )
    , tci.mips
    , VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    , VK_IMAGE_ASPECT_COLOR_BIT
}
, m_mipArray{ tci.mipArray }
{
    ZoneScoped;
    assert( tci.width > 0 );
    assert( tci.height > 0 );

    const VkSamplerCreateInfo samplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = addressMode( tci.u ),
        .addressModeV = addressMode( tci.v ),
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0f,
        .anisotropyEnable = tci.mips > 1,
        .maxAnisotropy = 16.0f,
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
    std::swap<Image>( *this, rhs );
    std::swap( m_sampler, rhs.m_sampler );
    std::swap( m_mipArray, rhs.m_mipArray );
}

TextureVK& TextureVK::operator = ( TextureVK&& rhs ) noexcept
{
    std::swap<Image>( *this, rhs );
    std::swap( m_sampler, rhs.m_sampler );
    std::swap( m_mipArray, rhs.m_mipArray );
    return *this;
}

struct RegionGenerator {
    const TextureCreateInfo::MipArray* mipArray = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipId = 0;

    VkBufferImageCopy operator () ();
};

VkBufferImageCopy RegionGenerator::operator () ()
{
    const uint32_t mipLevel = mipId++;
    return VkBufferImageCopy{
        .bufferOffset = std::get<0>( (*mipArray)[ mipLevel ] ),
        .imageSubresource = VkImageSubresourceLayers{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = mipLevel,
            .layerCount = 1,
        },
        .imageExtent = VkExtent3D{
            .width = width >> mipLevel,
            .height = height >> mipLevel,
            .depth = 1,
        },
    };
}


void TextureVK::transferFrom( VkCommandBuffer cmd, const BufferVK& buffer )
{
    ZoneScoped;
    transfer( cmd, constants::copyTo );

    std::pmr::vector<VkBufferImageCopy> regions( mipCount() );
    std::generate( regions.begin(), regions.end(), RegionGenerator{ &m_mipArray, m_extent.width, m_extent.height } );

    vkCmdCopyBufferToImage(
        cmd
        , buffer
        , m_image
        , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        , static_cast<uint32_t>( regions.size() )
        , regions.data()
    );
    transfer( cmd, constants::fragmentRead );
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
        .imageView = m_imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}
