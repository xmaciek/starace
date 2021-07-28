#include "render_target.hpp"

#include "utils_vk.hpp"

#include <cassert>
#include <utility>


void RenderTarget::destroyResources() noexcept
{
    destroy<vkDestroyFramebuffer, VkFramebuffer>( m_device, m_framebuffer );
    destroy<vkDestroyImageView, VkImageView>( m_device, m_imageView );
    destroy<vkDestroyImageView, VkImageView>( m_device, m_depthView );
    destroy<vkDestroyImage, VkImage>( m_device, m_image );
    destroy<vkDestroyImage, VkImage>( m_device, m_depth );
    destroy<vkFreeMemory, VkDeviceMemory>( m_device, m_imageMemory );
    destroy<vkFreeMemory, VkDeviceMemory>( m_device, m_depthMemory );
}

RenderTarget::~RenderTarget() noexcept
{
    destroyResources();
}

RenderTarget::RenderTarget(
    VkPhysicalDevice pdevice
    , VkDevice device
    , VkRenderPass renderPass
    , VkExtent2D extent2d
    , VkFormat imageFormat
    , VkFormat depthFormat
    ) noexcept
: m_device{ device }
, m_extent{ extent2d }
, m_imageFormat{ imageFormat }
, m_depthFormat{ depthFormat }
{
    assert( pdevice );
    assert( device );
    assert( extent2d.width > 0 );
    assert( extent2d.height > 0 );
    assert( imageFormat != VK_FORMAT_UNDEFINED );
    std::tie( m_image, m_imageView, m_imageMemory ) = createImage(
        pdevice,
        device,
        m_extent,
        m_imageFormat,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    );
    assert( m_image );
    assert( m_imageMemory );
    assert( m_imageView );

    std::tie( m_depth, m_depthView, m_depthMemory ) = createImage(
        pdevice,
        device,
        m_extent,
        m_depthFormat,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );
    assert( m_depth );
    assert( m_depthMemory );
    assert( m_depthView );

    const std::array attachments = { m_imageView, m_depthView };
    const VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .width = m_extent.width,
        .height = m_extent.height,
        .layers = 1,
    };

    [[maybe_unused]]
    const VkResult frameBufferOK = vkCreateFramebuffer( m_device, &framebufferInfo, nullptr, &m_framebuffer );
    assert( frameBufferOK == VK_SUCCESS );
}

RenderTarget::RenderTarget( RenderTarget&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_framebuffer, rhs.m_framebuffer );
    std::swap( m_extent, rhs.m_extent );

    std::swap( m_image, rhs.m_image );
    std::swap( m_imageView, rhs.m_imageView );
    std::swap( m_imageMemory, rhs.m_imageMemory );
    std::swap( m_imageFormat, rhs.m_imageFormat );
    std::swap( m_imageStage, rhs.m_imageStage );
    std::swap( m_imageAccess, rhs.m_imageAccess );

    std::swap( m_depth, rhs.m_depth );
    std::swap( m_depthView, rhs.m_depthView );
    std::swap( m_depthMemory, rhs.m_depthMemory );
    std::swap( m_depthFormat, rhs.m_depthFormat );
    std::swap( m_depthStage, rhs.m_depthStage );
    std::swap( m_depthAccess, rhs.m_depthAccess );
}

RenderTarget& RenderTarget::operator = ( RenderTarget&& rhs ) noexcept
{
    destroyResources();
    moveClear( m_device, rhs.m_device );
    moveClear( m_framebuffer, rhs.m_framebuffer );
    moveClear( m_extent, rhs.m_extent );

    moveClear( m_image, rhs.m_image );
    moveClear( m_imageView, rhs.m_imageView );
    moveClear( m_imageMemory, rhs.m_imageMemory );
    moveClear( m_imageFormat, rhs.m_imageFormat );
    moveClear( m_imageStage, rhs.m_imageStage );
    moveClear( m_imageAccess, rhs.m_imageAccess );

    moveClear( m_depth, rhs.m_depth );
    moveClear( m_depthView, rhs.m_depthView );
    moveClear( m_depthMemory, rhs.m_depthMemory );
    moveClear( m_depthFormat, rhs.m_depthFormat );
    moveClear( m_depthStage, rhs.m_depthStage );
    moveClear( m_depthAccess, rhs.m_depthAccess );

    return *this;
}

void RenderTarget::transferToWrite( VkCommandBuffer cmd )
{
    assert( cmd );

    const TransferInfo src{
        .m_layout = m_imageLayout,
        .m_access = m_imageAccess,
        .m_stage = m_imageStage,
    };

    static constexpr TransferInfo dst{
        .m_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .m_access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .m_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    transferImage( cmd, m_image, src, dst );

    m_imageLayout = dst.m_layout;
    m_imageAccess = dst.m_access;
    m_imageStage = dst.m_stage;
}


void RenderTarget::transferToRead( VkCommandBuffer cmd )
{
    assert( cmd );

    pretendIsWrite();
    const TransferInfo src{
        .m_layout = m_imageLayout,
        .m_access = m_imageAccess,
        .m_stage = m_imageStage,
    };

    static constexpr TransferInfo dst{
        .m_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .m_access = VK_ACCESS_TRANSFER_READ_BIT,
        .m_stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
    };
    transferImage( cmd, m_image, src, dst );

    m_imageLayout = dst.m_layout;
    m_imageAccess = dst.m_access;
    m_imageStage = dst.m_stage;
}

void RenderTarget::pretendIsWrite()
{
    m_imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_imageAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    m_imageStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
}

std::pair<VkImage, VkImageView> RenderTarget::image() const
{
    assert( m_image );
    assert( m_imageView );
    return { m_image, m_imageView };
}

VkFramebuffer RenderTarget::framebuffer() const
{
    assert( m_framebuffer );
    return m_framebuffer;
}

VkRect2D RenderTarget::rect() const
{
    return { .extent = m_extent };
}

VkExtent2D RenderTarget::extent() const
{
    return m_extent;
}

VkImageLayout RenderTarget::layout() const
{
    return m_imageLayout;
}
