#include "renderpass.hpp"

#include <array>
#include <cassert>
#include <utility>

#include "utils_vk.hpp"

#include <Tracy.hpp>

RenderPass::RenderPass( VkDevice device, VkFormat format, VkFormat depthFormat ) noexcept
: m_device{ device }
{
    ZoneScoped;
    static constexpr VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    static constexpr VkAttachmentReference depthAttachmentRef{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    static constexpr VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };

    const VkAttachmentDescription colorAttachmentClear{
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentDescription colorAttachmentLoad{
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentDescription depthAttachment{
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    std::array attachments = { colorAttachmentLoad, depthAttachment };
    const VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    [[maybe_unused]]
    VkResult res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderPass );
    assert( res == VK_SUCCESS );

    attachments[ 0 ] = colorAttachmentClear;
    res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderClear );
    assert( res == VK_SUCCESS );

    m_currentPass = m_renderClear;
}

RenderPass::RenderPass( VkDevice device, VkFormat depthFormat ) noexcept
: m_device{ device }
, m_depthOnly{ true }
{
    ZoneScoped;
    static constexpr VkAttachmentReference depthAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    static constexpr VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .pDepthStencilAttachment = &depthAttachmentRef,
    };


    const VkAttachmentDescription depthAttachmentClear{
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    const VkAttachmentDescription depthAttachmentLoad{
        .format = depthFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
    };

    std::array attachments = { depthAttachmentLoad };
    const VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    [[maybe_unused]]
    VkResult res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderPass );
    assert( res == VK_SUCCESS );

    attachments[ 0 ] = depthAttachmentClear;
    res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderClear );
    assert( res == VK_SUCCESS );

    m_currentPass = m_renderClear;
}

RenderPass::~RenderPass() noexcept
{
    destroy<vkDestroyRenderPass, VkRenderPass>( m_device, m_renderPass );
    destroy<vkDestroyRenderPass, VkRenderPass>( m_device, m_renderClear );
}

RenderPass::RenderPass( RenderPass&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_renderClear, rhs.m_renderClear );
    std::swap( m_renderPass, rhs.m_renderPass );
    std::swap( m_currentPass, rhs.m_currentPass );
    std::swap( m_depthOnly, rhs.m_depthOnly );
}

RenderPass& RenderPass::operator = ( RenderPass&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_renderClear, rhs.m_renderClear );
    std::swap( m_renderPass, rhs.m_renderPass );
    std::swap( m_currentPass, rhs.m_currentPass );
    std::swap( m_depthOnly, rhs.m_depthOnly );
    return *this;
}

RenderPass::operator VkRenderPass () const noexcept
{
    assert( m_currentPass );
    return m_currentPass;
}

void RenderPass::begin( VkCommandBuffer cmd, VkFramebuffer framebuffer, const VkRect2D& renderArea ) noexcept
{
    assert( m_renderClear );
    assert( cmd );
    assert( framebuffer );
    static constexpr std::array clearColor{
        VkClearValue{ .color = { .float32{ 0.0f, 0.0f, 0.0f, 0.0f } } },
        VkClearValue{ .depthStencil = { 1.0f, 0u } }
    };

    const VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = (float)renderArea.extent.width,
        .height = (float)renderArea.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport( cmd, 0, 1, &viewport );
    vkCmdSetScissor( cmd, 0, 1, &renderArea );

    const VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderClear,
        .framebuffer = framebuffer,
        .renderArea = renderArea,
        .clearValueCount = m_depthOnly ? 1u : 2u,
        .pClearValues = m_depthOnly ? clearColor.data() + 1: clearColor.data(),
    };
    vkCmdBeginRenderPass( cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void RenderPass::resume( VkCommandBuffer cmd, VkFramebuffer framebuffer, const VkRect2D& renderArea ) noexcept
{
    assert( m_renderPass );
    assert( cmd );
    assert( framebuffer );

    const VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = framebuffer,
        .renderArea = renderArea,
    };
    vkCmdBeginRenderPass( cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    m_currentPass = m_renderPass;
}
void RenderPass::end( VkCommandBuffer cmd ) noexcept
{
    assert( cmd );
    vkCmdEndRenderPass( cmd );
}
