#include "renderpass.hpp"

#include <array>
#include <cassert>
#include <utility>

#include "utils_vk.hpp"

#include <profiler.hpp>

RenderPass::RenderPass( Purpose purpose ) noexcept
: m_depthOnly{ purpose == eDepth }
{
}

void RenderPass::begin( VkCommandBuffer cmd, const Image& depth, const Image& color ) noexcept
{
    assert( cmd );
    static constexpr std::array clearColor{
        VkClearValue{ .color = { .float32{ 0.0f, 0.0f, 0.0f, 0.0f } } },
        VkClearValue{ .depthStencil = { 1.0f, 0u } }
    };

    const VkExtent2D extent = depth.extent();
    const VkRect2D rect{ {}, extent };
    const VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    vkCmdSetViewport( cmd, 0, 1, &viewport );
    vkCmdSetScissor( cmd, 0, 1, &rect );

    const VkRenderingAttachmentInfoKHR colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = color.view(),
        .imageLayout = constants::fragmentWrite.m_layout,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };
    const VkRenderingAttachmentInfoKHR depthAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depth.view(),
        .imageLayout = m_depthOnly ? constants::depthWrite.m_layout : constants::depthRead.m_layout,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = m_depthOnly ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = m_depthOnly ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_NONE_KHR,
        .clearValue = clearColor[ m_depthOnly ],
    };
    const VkRenderingInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = rect,
        .layerCount = 1u,
        .colorAttachmentCount = m_depthOnly ? 0u : 1u,
        .pColorAttachments = m_depthOnly ? nullptr : &colorAttachment,
        .pDepthAttachment = &depthAttachment,
    };
    vkCmdBeginRenderingKHR( cmd, &renderInfo );
}

void RenderPass::resume( VkCommandBuffer cmd, const Image& depth, const Image& color ) noexcept
{
    assert( cmd );
    const VkRenderingAttachmentInfoKHR colorAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_depthOnly ? VkImageView{} : color.view(),
        .imageLayout = constants::fragmentWrite.m_layout,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };
    const VkRenderingAttachmentInfoKHR depthAttachment{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depth.view(),
        .imageLayout = m_depthOnly ? constants::depthWrite.m_layout : constants::depthWrite.m_layout,
        .resolveMode = VK_RESOLVE_MODE_NONE,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = m_depthOnly ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_NONE_KHR,
    };
    const VkRenderingInfo renderInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .flags = VK_RENDERING_RESUMING_BIT,
        .renderArea = VkRect2D{ {}, depth.extent() },
        .layerCount = 1u,
        .colorAttachmentCount = m_depthOnly ? 0u : 1u,
        .pColorAttachments = m_depthOnly ? nullptr : &colorAttachment,
        .pDepthAttachment = &depthAttachment,
    };
    vkCmdBeginRenderingKHR( cmd, &renderInfo );
}

void RenderPass::end( VkCommandBuffer cmd ) noexcept
{
    assert( cmd );
    vkCmdEndRenderingKHR( cmd );
}
