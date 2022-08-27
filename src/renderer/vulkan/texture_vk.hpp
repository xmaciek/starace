#pragma once

#include "buffer_vk.hpp"
#include "image.hpp"

#include <renderer/texture.hpp>

#include <vulkan/vulkan.h>

class TextureVK : public Image {
    VkSampler m_sampler = VK_NULL_HANDLE;
    TextureCreateInfo::MipArray m_mipArray{};

public:
    ~TextureVK();
    TextureVK() = default;
    TextureVK( VkPhysicalDevice, VkDevice, VkExtent2D, VkFormat );
    TextureVK( const TextureCreateInfo&, VkPhysicalDevice, VkDevice );

    TextureVK( TextureVK&& ) noexcept;
    TextureVK& operator = ( TextureVK&& ) noexcept;

    void transferFrom( VkCommandBuffer, const BufferVK& );

    VkSampler sampler() const;
    VkDescriptorImageInfo imageInfo() const;

};
