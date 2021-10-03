#pragma once

#include "buffer_vk.hpp"

#include <vulkan/vulkan.h>

class TextureVK {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkExtent2D m_extent{};
    VkImage m_image = VK_NULL_HANDLE;
    VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageView m_view = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    VkAccessFlags m_currentAccess = 0;
    VkPipelineStageFlags m_currentStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    void destroyResources();
    void transitionLayout( VkCommandBuffer, VkImageLayout );

public:
    ~TextureVK();
    TextureVK() = default;
    TextureVK( VkPhysicalDevice, VkDevice, VkExtent2D, VkFormat );

    TextureVK( TextureVK&& ) noexcept;
    TextureVK& operator = ( TextureVK&& ) noexcept;

    void transferFrom( VkCommandBuffer, const BufferVK& );

    VkImageView view() const;
    VkSampler sampler() const;
    VkDescriptorImageInfo imageInfo() const;

};
