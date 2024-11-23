#include "texture_vk.hpp"

#include "utils_vk.hpp"

#include <profiler.hpp>

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>
#include <memory_resource>

static auto format( const TextureCreateInfo& tci )
{
    switch ( tci.format ) {
    case TextureFormat::eR: return VK_FORMAT_R8_UNORM;
    case TextureFormat::eBGRA: return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::eBC1_unorm: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case TextureFormat::eBC2_unorm: return VK_FORMAT_BC2_UNORM_BLOCK;
    case TextureFormat::eBC3_unorm: return VK_FORMAT_BC3_UNORM_BLOCK;
    case TextureFormat::eBC4_unorm: return VK_FORMAT_BC4_UNORM_BLOCK;
    case TextureFormat::eBC5_unorm: return VK_FORMAT_BC5_UNORM_BLOCK;
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

static uint32_t formatToChannels( VkFormat f )
{
    switch ( f ) {
    case VK_FORMAT_R8_UNORM:
    case VK_FORMAT_BC4_UNORM_BLOCK:
        return 1;
    case VK_FORMAT_B8G8R8A8_UNORM:
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
    case VK_FORMAT_BC2_UNORM_BLOCK:
    case VK_FORMAT_BC3_UNORM_BLOCK:
        return 4;
    case VK_FORMAT_BC5_UNORM_BLOCK:
        return 2;
    default:
        assert( !"unhandled formatToChannels case" );
        return 0;
    }
}

TextureVK::~TextureVK()
{
    destroy<vkDestroySampler>( m_device, m_sampler );
}

TextureVK::TextureVK( const TextureCreateInfo& tci, VkPhysicalDevice physDevice, VkDevice device )
: Image{
    physDevice
    , device
    , { .width = tci.width, .height = tci.height }
    , format( tci )
    , tci.mips
    , tci.array
    , VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    , VK_IMAGE_ASPECT_COLOR_BIT
}
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
    m_channels = formatToChannels( m_format );
}

TextureVK::TextureVK( TextureVK&& rhs ) noexcept
{
    std::swap<Image>( *this, rhs );
    std::swap( m_sampler, rhs.m_sampler );
    std::swap( m_channels, rhs.m_channels );
}

TextureVK& TextureVK::operator = ( TextureVK&& rhs ) noexcept
{
    std::swap<Image>( *this, rhs );
    std::swap( m_sampler, rhs.m_sampler );
    std::swap( m_channels, rhs.m_channels );
    return *this;
}

struct RegionGenerator {
    uint32_t m_bufferSize = 0;
    uint32_t m_mipSize = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_mips = 0;
    uint32_t m_arrays = 0;

    uint32_t m_offset = 0;
    uint32_t m_id = 0;

    VkBufferImageCopy operator () ();
};

VkBufferImageCopy RegionGenerator::operator () ()
{
    const uint32_t bufferOffset = m_offset;
    const uint32_t id = m_id++;
    const uint32_t mipLevel = id % m_mips;
    const uint32_t arrayId = id / m_mips;
    m_offset += m_mipSize >> ( mipLevel * 2 );
    assert( m_offset <= m_bufferSize );
    assert( arrayId < m_arrays );

    return VkBufferImageCopy{
        .bufferOffset = bufferOffset,
        .imageSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = mipLevel,
            .baseArrayLayer = arrayId,
            .layerCount = 1,
        },
        .imageExtent = VkExtent3D{
            .width = m_width >> mipLevel,
            .height = m_height >> mipLevel,
            .depth = 1,
        },
    };
}

void TextureVK::transferFrom( VkCommandBuffer cmd, const BufferVK& buffer, uint32_t mip0ByteCount )
{
    ZoneScoped;
    transfer( cmd, constants::copyTo );

    RegionGenerator regionGen{
        .m_bufferSize = buffer.sizeInBytes(),
        .m_mipSize = mip0ByteCount,
        .m_width = m_extent.width,
        .m_height = m_extent.height,
        .m_mips = mipCount(),
        .m_arrays = arrayCount(),
    };
    std::pmr::vector<VkBufferImageCopy> regions( mipCount() * arrayCount() );
    std::generate( regions.begin(), regions.end(), regionGen );

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
