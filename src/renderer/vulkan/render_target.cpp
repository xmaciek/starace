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
    Purpose purpose
    , VkPhysicalDevice pdevice
    , VkDevice device
    , VkRenderPass renderPass
    , VkExtent2D extent
    , VkFormat depthFormat
    , VkImageView extraView
) noexcept
: Image{ pdevice, device, extent, depthFormat, 1
    , std::get<0>( purpose )
    , std::get<1>( purpose )
    , std::get<2>( purpose )
}
{
    ZoneScoped;
    assert( pdevice );
    assert( device );

    const std::array attachments = { view(), extraView };
    const VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = extraView ? (uint32_t)attachments.size() : 1u,
        .pAttachments = attachments.data(),
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    [[maybe_unused]]
    const VkResult frameBufferOK = vkCreateFramebuffer( m_device, &framebufferInfo, nullptr, &m_framebuffer );
    assert( frameBufferOK == VK_SUCCESS );
}

RenderTarget::RenderTarget( RenderTarget&& rhs ) noexcept
{
    std::swap<Image>( *this, rhs );
    std::swap( m_framebuffer, rhs.m_framebuffer );
}

RenderTarget& RenderTarget::operator = ( RenderTarget&& rhs ) noexcept
{
    std::swap<Image>( *this, rhs );
    std::swap( m_framebuffer, rhs.m_framebuffer );
    return *this;
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

VkExtent3D RenderTarget::extent3D() const
{
    return {
        .width = m_extent.width,
        .height = m_extent.height,
        .depth = 1,
    };
}
