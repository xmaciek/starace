#pragma once

#include <renderer/pipeline.hpp>
#include "buffer_vk.hpp"
#include "descriptor_set.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <string_view>
#include <vector>

class PipelineVK {
public:
    using DescriptorWrites = std::array<VkWriteDescriptorSet, 8>;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipeline m_pipelineDepthPrepass = VK_NULL_HANDLE;
    DescriptorWrites m_descriptorWrites{};
    uint32_t m_pushConstantSize = 0;
    uint32_t m_vertexStride = 0;
    uint32_t m_descriptorSetId = 0;
    uint32_t m_descriptorWriteCount = 0;
    bool m_depthWrite = false;
    bool m_useLines = false;

public:
    ~PipelineVK() noexcept;
    PipelineVK() noexcept = default;

    PipelineVK(
        const PipelineCreateInfo&
        , VkDevice
        , VkRenderPass color
        , VkRenderPass depth
        , VkDescriptorSetLayout
        , uint32_t descriptorSetId
    ) noexcept;

    PipelineVK(
        const PipelineCreateInfo& pci
        , VkDevice device
        , VkDescriptorSetLayout layout
        , uint32_t descriptorSetId
    ) noexcept;

    PipelineVK( PipelineVK&& ) noexcept;
    PipelineVK& operator = ( PipelineVK&& ) noexcept;

    operator VkPipeline () const;
    VkPipeline depthPrepass() const;

    VkPipelineLayout layout() const;
    uint32_t pushConstantSize() const;
    uint32_t vertexStride() const;
    uint32_t descriptorSetId() const;
    uint32_t descriptorWriteCount() const;
    DescriptorWrites descriptorWrites() const;

    bool depthWrite() const;
    bool useLines() const;
};
