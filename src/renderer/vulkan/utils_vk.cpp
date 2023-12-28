#include "utils_vk.hpp"

#include <cassert>

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

void transferImage( VkCommandBuffer cmd, VkImage image, const TransferInfo& src, const TransferInfo& dst, uint32_t mipCount, uint32_t arrayCount )
{
    assert( mipCount > 0 );
    const VkImageSubresourceRange imageSubresourceRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = mipCount,
        .baseArrayLayer = 0,
        .layerCount = arrayCount,
    };

    assert( cmd );
    const VkImageMemoryBarrier imageBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src.m_access,
        .dstAccessMask = dst.m_access,
        .oldLayout = src.m_layout,
        .newLayout = dst.m_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = imageSubresourceRange,
    };

    vkCmdPipelineBarrier( cmd, src.m_stage, dst.m_stage, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier );

}

