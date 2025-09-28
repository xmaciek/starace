#pragma once

#include "buffer_vk.hpp"
#include "descriptor_set.hpp"
#include "device.hpp"
#include "vk.hpp"

#include <renderer/pipeline.hpp>

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
    std::array<VkWriteDescriptorSet, 2> m_descriptorWrites{};
    uint32_t m_pushConstantSize = 0;
    uint32_t m_vertexStride = 0;
    uint32_t m_descriptorSetId = 0;
    bool m_depthWrite = false;
    bool m_useLines = false;

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
    uint32_t pushConstantSize() const;
    uint32_t vertexStride() const;
    uint32_t descriptorSetId() const;
    uint32_t descriptorWriteCount() const;
    uint32_t descriptorWriteOffset() const;
    inline auto descriptorWrites() const { return m_descriptorWrites; }

    bool depthWrite() const;
    bool useLines() const;
};
