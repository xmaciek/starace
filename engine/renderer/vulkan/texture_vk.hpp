#pragma once

#include "buffer_vk.hpp"
#include "image.hpp"
#include "vk.hpp"

#include <renderer/texture.hpp>


class TextureVK : public Image {
    VkSampler m_sampler = VK_NULL_HANDLE;
    uint32_t m_channels = 0;

public:
    ~TextureVK();
    TextureVK() = default;
    TextureVK( const TextureCreateInfo&, VkPhysicalDevice, VkDevice );

    TextureVK( TextureVK&& ) noexcept;
    TextureVK& operator = ( TextureVK&& ) noexcept;

    void transferFrom( VkCommandBuffer, const BufferVK&, uint32_t mip0ByteCount );

    VkSampler sampler() const;
    VkDescriptorImageInfo imageInfo() const;
    inline uint32_t channels() const { return m_channels; }

};
