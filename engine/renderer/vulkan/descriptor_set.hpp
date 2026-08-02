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

    StackVector<VkWriteDescriptorSet, 8> m_descriptorWrites{};
    StackVector<VkDescriptorSetLayoutBinding, 8> m_bindingInfo{};
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

    static uint64_t createBindingID( const PipelineCreateInfo& );

    void update( VkDescriptorSet
        , std::span<const VkDescriptorBufferInfo> uniform
        , std::span<const VkDescriptorImageInfo> image
        , std::span<const VkDescriptorBufferInfo> storage
    );
};
