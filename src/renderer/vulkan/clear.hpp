#pragma once

#include <vulkan/vulkan.h>


class Clear {
    VkDevice m_device = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;

public:
    Clear() noexcept = default;
    Clear( VkDevice, VkFormat, VkFormat, bool doTransfer ) noexcept;
    ~Clear() noexcept;

    Clear( const Clear& ) = delete;
    Clear& operator = ( const Clear& ) = delete;
    Clear( Clear&& ) noexcept;
    Clear& operator = ( Clear&& ) noexcept;

    void operator () ( VkCommandBuffer, VkFramebuffer, const VkRect2D& ) noexcept;
};
