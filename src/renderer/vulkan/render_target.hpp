#pragma once

#include "image.hpp"

#include <vulkan/vulkan.h>

#include <utility>

class RenderTarget {
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkExtent2D m_extent{};

    Image m_color{};
    Image m_depth{};

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
