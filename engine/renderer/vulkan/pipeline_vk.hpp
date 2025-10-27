#pragma once

#include "buffer_vk.hpp"
#include "descriptor_set.hpp"
#include "device.hpp"
#include "vk.hpp"

#include <renderer/pipeline.hpp>
#include <shared/stack_vector.hpp>

#include <cstdint>
#include <memory_resource>
#include <string_view>
#include <vector>

class PipelineVK {
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipeline m_pipelineDepthPrepass = VK_NULL_HANDLE;
    StackVector<VkWriteDescriptorSet, 2> m_descriptorWrites{};
    uint32_t m_vertexStride = 0;
    uint32_t m_descriptorSetId = 0;
    bool m_depthWrite = false;
    bool m_useLines = false;
    bool m_hasUniform = false;
    bool m_hasImage = false;

public:
    ~PipelineVK() noexcept;
    PipelineVK() noexcept = default;

    PipelineVK(
        const PipelineCreateInfo&
        , const Device&
        , VkFormat depthFormat
        , VkFormat colorFormat
        , VkDescriptorSetLayout
        , uint32_t descriptorSetId
    ) noexcept;

    PipelineVK( PipelineVK&& ) noexcept;
    PipelineVK& operator = ( PipelineVK&& ) noexcept;

    operator VkPipeline () const;
    VkPipeline depthPrepass() const;

    VkPipelineLayout layout() const;
    uint32_t vertexStride() const;
    uint32_t descriptorSetId() const;
    void updateDescriptorSet( VkDescriptorSet, const VkDescriptorBufferInfo&, std::span<const VkDescriptorImageInfo> );

    bool depthWrite() const;
    bool useLines() const;
};
