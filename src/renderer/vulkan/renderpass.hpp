#pragma once

#include "vk.hpp"

class RenderPass {
    VkDevice m_device = VK_NULL_HANDLE;
    VkRenderPass m_renderClear = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkRenderPass m_currentPass = VK_NULL_HANDLE;
    bool m_depthOnly = false;

public:
    RenderPass() noexcept = default;
    RenderPass( VkDevice, VkFormat, VkFormat ) noexcept;
    RenderPass( VkDevice, VkFormat ) noexcept;
    ~RenderPass() noexcept;

    RenderPass( const RenderPass& ) = delete;
    RenderPass& operator = ( const RenderPass& ) = delete;
    RenderPass( RenderPass&& ) noexcept;
    RenderPass& operator = ( RenderPass&& ) noexcept;

    operator VkRenderPass() const noexcept;

    void begin( VkCommandBuffer, VkFramebuffer, const VkRect2D& ) noexcept;
    void resume( VkCommandBuffer, VkFramebuffer, const VkRect2D& ) noexcept;
    void end( VkCommandBuffer ) noexcept;

};
