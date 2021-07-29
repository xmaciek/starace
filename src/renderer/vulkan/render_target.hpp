#pragma once

#include <vulkan/vulkan.h>

#include <utility>

class RenderTarget {
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkExtent2D m_extent = {};

    VkImage m_image = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
    VkFormat m_imageFormat = VK_FORMAT_UNDEFINED;

    VkImage m_depth = VK_NULL_HANDLE;
    VkImageView m_depthView = VK_NULL_HANDLE;
    VkDeviceMemory m_depthMemory = VK_NULL_HANDLE;
    VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;

    void destroyResources() noexcept;

public:
    ~RenderTarget() noexcept;
    RenderTarget() noexcept = default;
    RenderTarget( VkPhysicalDevice, VkDevice, VkRenderPass, VkExtent2D, VkFormat imageFormat, VkFormat depthFormat ) noexcept;

    RenderTarget( const RenderTarget& ) = delete;
    RenderTarget& operator = ( const RenderTarget& ) = delete;

    RenderTarget( RenderTarget&& ) noexcept;
    RenderTarget& operator = ( RenderTarget&& ) noexcept;

    std::pair<VkImage, VkImageView> image() const;
    VkFramebuffer framebuffer() const;

    VkRect2D rect() const;
    VkExtent2D extent() const;
    VkExtent3D extent3D() const;
};
