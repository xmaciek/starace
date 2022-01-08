#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <utility>
#include <vector>

class DescriptorSet {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    std::pmr::vector<VkDescriptorSet> m_set;
    uint32_t m_current = 0;

    void destroyResources();

public:
    ~DescriptorSet() noexcept;
    DescriptorSet() noexcept = default;

    DescriptorSet(
        VkDevice
        , uint32_t setsPerFrame
        , uint16_t constantBindBits
        , uint16_t samplerBindBits
    ) noexcept;

    DescriptorSet( DescriptorSet&& ) noexcept;
    DescriptorSet& operator = ( DescriptorSet&& ) noexcept;

    VkDescriptorSetLayout layout() const;
    VkDescriptorSet next();
    void reset();
};
