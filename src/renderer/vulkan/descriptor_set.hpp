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
    using UniformObject = std::pair<VkDescriptorType, VkShaderStageFlagBits>;
    static constexpr UniformObject uniformBuffer = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT };
    static constexpr UniformObject imageSampler = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT };


    ~DescriptorSet();
    DescriptorSet() = default;
    DescriptorSet(
        VkDevice device
        , uint32_t setsPerFrame
        , const std::pmr::vector<UniformObject>& types
    );

    DescriptorSet( DescriptorSet&& ) noexcept;
    DescriptorSet& operator = ( DescriptorSet&& ) noexcept;

    VkDescriptorSetLayout layout() const;
    VkDescriptorSet next();
    void reset();
};
