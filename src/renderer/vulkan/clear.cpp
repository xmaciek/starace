#include "clear.hpp"

#include <cassert>
#include <utility>

Clear::Clear( VkDevice device, VkFormat format, bool doTransfer ) noexcept
: m_device{ device }
{
    static constexpr VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    static constexpr VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    static constexpr VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    const VkAttachmentDescription colorAttachment{
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = doTransfer ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = doTransfer ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = doTransfer ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    [[maybe_unused]]
    const VkResult res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderPass );
    assert( res == VK_SUCCESS );
}

Clear::~Clear() noexcept
{
    if ( m_renderPass ) {
        vkDestroyRenderPass( m_device, m_renderPass, nullptr );
    }
}

Clear::Clear( Clear&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_renderPass, rhs.m_renderPass );
}

Clear& Clear::operator = ( Clear&& rhs ) noexcept
{
    if ( m_renderPass ) {
        vkDestroyRenderPass( m_device, m_renderPass, nullptr );
    }
    m_renderPass = rhs.m_renderPass;
    m_device = rhs.m_device;
    rhs.m_renderPass = VK_NULL_HANDLE;
    rhs.m_device = VK_NULL_HANDLE;
    return *this;
}

void Clear::operator () ( VkCommandBuffer cmd, VkFramebuffer framebuffer, const VkRect2D& renderArea ) noexcept
{
    assert( m_renderPass );
    const VkClearValue clearColor{ VkClearColorValue{ { 0.0f, 0.0f, 1.0f, 1.0f } } };
    const VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = framebuffer,
        .renderArea = renderArea,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    vkCmdBeginRenderPass( cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    vkCmdEndRenderPass( cmd );
}
