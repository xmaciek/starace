#pragma once

#include "vk.hpp"

#include <renderer/pipeline.hpp>

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
    uint32_t m_uniformCount = 0;
    uint32_t m_imagesCount = 0;
    VkDescriptorType m_imageType{};

    void expandCapacityBy( uint32_t );

public:
    ~DescriptorSet() noexcept;
    DescriptorSet() noexcept = default;

    DescriptorSet( VkDevice, const PipelineCreateInfo& ) noexcept;
    DescriptorSet( DescriptorSet&& ) noexcept;

    DescriptorSet& operator = ( DescriptorSet&& ) noexcept;

    VkDescriptorSetLayout layout() const;
    VkDescriptorSet next();
    void reset();
};
