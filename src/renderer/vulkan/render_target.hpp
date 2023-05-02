#pragma once

#include "image.hpp"
#include "vk.hpp"

#include <tuple>

class RenderTarget : public Image {
private:
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

public:
    struct Purpose {
        uint32_t usage = 0;
        VkMemoryPropertyFlagBits memoryFlags{};
        VkImageAspectFlagBits aspectFlags{};
    };
    static constexpr inline Purpose COLOR{
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        .memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
    };
    static constexpr inline Purpose DEPTH{
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
    };

    ~RenderTarget() noexcept;
    RenderTarget() noexcept = default;
    RenderTarget( const Purpose&, VkPhysicalDevice, VkDevice, VkRenderPass, VkExtent2D, VkFormat, VkImageView extraView ) noexcept;

    RenderTarget( const RenderTarget& ) = delete;
    RenderTarget& operator = ( const RenderTarget& ) = delete;

    RenderTarget( RenderTarget&& ) noexcept;
    RenderTarget& operator = ( RenderTarget&& ) noexcept;

    VkFramebuffer framebuffer() const;

    VkRect2D rect() const;
    VkExtent3D extent3D() const;
    VkDescriptorImageInfo imageInfo() const;
};
