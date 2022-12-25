#pragma once

#include "image.hpp"
#include "vk.hpp"

#include <tuple>

class RenderTarget : public Image {
private:
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

public:
    using Purpose = std::tuple<uint32_t, VkMemoryPropertyFlagBits, VkImageAspectFlagBits>;
    static constexpr Purpose c_color{
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
    };
    static constexpr Purpose c_depth{
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT,
    };

    ~RenderTarget() noexcept;
    RenderTarget() noexcept = default;
    RenderTarget( Purpose, VkPhysicalDevice, VkDevice, VkRenderPass, VkExtent2D, VkFormat, VkImageView extraView ) noexcept;

    RenderTarget( const RenderTarget& ) = delete;
    RenderTarget& operator = ( const RenderTarget& ) = delete;

    RenderTarget( RenderTarget&& ) noexcept;
    RenderTarget& operator = ( RenderTarget&& ) noexcept;

    VkFramebuffer framebuffer() const;

    VkRect2D rect() const;
    VkExtent3D extent3D() const;
    VkDescriptorImageInfo imageInfo() const;
};
