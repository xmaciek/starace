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

    std::swap( m_depth, rhs.m_depth );
    std::swap( m_depthView, rhs.m_depthView );
    std::swap( m_depthMemory, rhs.m_depthMemory );
    std::swap( m_depthFormat, rhs.m_depthFormat );
}

RenderTarget& RenderTarget::operator = ( RenderTarget&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_framebuffer = std::exchange( rhs.m_framebuffer, {} );
    m_extent = std::exchange( rhs.m_extent, {} );

    m_image = std::exchange( rhs.m_image, {} );
    m_imageView = std::exchange( rhs.m_imageView, {} );
    m_imageMemory = std::exchange( rhs.m_imageMemory, {} );
    m_imageFormat = std::exchange( rhs.m_imageFormat, {} );

    m_depth = std::exchange( rhs.m_depth, {} );
    m_depthView = std::exchange( rhs.m_depthView, {} );
    m_depthMemory = std::exchange( rhs.m_depthMemory, {} );
    m_depthFormat = std::exchange( rhs.m_depthFormat, {} );

    return *this;
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

VkExtent3D RenderTarget::extent3D() const
{
    return {
        .width = m_extent.width,
        .height = m_extent.height,
        .depth = 1,
    };
}
