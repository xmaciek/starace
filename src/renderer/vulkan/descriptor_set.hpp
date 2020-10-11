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
    ~DescriptorSet();
    DescriptorSet() = default;
    DescriptorSet(
        VkDevice device
        , uint32_t swapchainCount
        , uint32_t setsPerFrame
        , const std::pmr::vector<std::pair<VkDescriptorType, VkShaderStageFlagBits>>& types
    );

    DescriptorSet( DescriptorSet&& ) noexcept;
    DescriptorSet& operator = ( DescriptorSet&& ) noexcept;

    const VkDescriptorSetLayout* layout() const;
    VkDescriptorSet next();
    void reset();
};
