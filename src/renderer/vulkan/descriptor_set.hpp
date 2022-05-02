#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <utility>
#include <vector>

class DescriptorSet {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
    std::pmr::vector<VkDescriptorPool> m_pools{};
    std::pmr::vector<VkDescriptorSet> m_set;
    uint32_t m_current = 0;
    uint32_t m_imagesCount = 0;
    bool m_isGraphics = true;

    void expandCapacityBy( uint32_t );

public:
    ~DescriptorSet() noexcept;
    DescriptorSet() noexcept = default;

    DescriptorSet(
        VkDevice
        , uint16_t constantBindBits
        , uint16_t samplerBindBits
        , uint16_t computeImageBindBits
    ) noexcept;

    DescriptorSet( DescriptorSet&& ) noexcept;
    DescriptorSet& operator = ( DescriptorSet&& ) noexcept;

    VkDescriptorSetLayout layout() const;
    VkDescriptorSet next();
    void reset();
};
