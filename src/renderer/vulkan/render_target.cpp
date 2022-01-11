#include "render_target.hpp"

#include "utils_vk.hpp"

#include <Tracy.hpp>

#include <cassert>
#include <utility>

RenderTarget::~RenderTarget() noexcept
{
    destroy<vkDestroyFramebuffer, VkFramebuffer>( m_device, m_framebuffer );
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
, m_color{ pdevice, device, extent2d, imageFormat, 1
    , VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    , VK_IMAGE_ASPECT_COLOR_BIT
}
, m_depth{ pdevice, device, extent2d, depthFormat, 1
    , VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    , VK_IMAGE_ASPECT_DEPTH_BIT
}
{
    ZoneScoped;
    assert( pdevice );
    assert( device );

    const std::array attachments = { m_color.view(), m_depth.view() };
    const VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .width = extent2d.width,
        .height = extent2d.height,
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
    std::swap( m_color, rhs.m_color );
    std::swap( m_depth, rhs.m_depth );
}

RenderTarget& RenderTarget::operator = ( RenderTarget&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_framebuffer, rhs.m_framebuffer );
    std::swap( m_extent, rhs.m_extent );
    std::swap( m_color, rhs.m_color );
    std::swap( m_depth, rhs.m_depth );
    return *this;
}

std::pair<VkImage, VkImageView> RenderTarget::image() const
{
    return { m_color.image(), m_color.view() };
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
