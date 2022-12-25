#pragma once

#include "buffer_vk.hpp"
#include "image.hpp"
#include "vk.hpp"

#include <renderer/texture.hpp>


class TextureVK : public Image {
    VkSampler m_sampler = VK_NULL_HANDLE;
    TextureCreateInfo::MipArray m_mipArray{};

public:
    ~TextureVK();
    TextureVK() = default;
    TextureVK( const TextureCreateInfo&, VkPhysicalDevice, VkDevice );

    TextureVK( TextureVK&& ) noexcept;
    TextureVK& operator = ( TextureVK&& ) noexcept;

    void transferFrom( VkCommandBuffer, const BufferVK& );

    VkSampler sampler() const;
    VkDescriptorImageInfo imageInfo() const;

};
