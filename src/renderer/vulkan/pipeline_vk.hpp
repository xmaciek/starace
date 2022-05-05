#pragma once

#include <renderer/pipeline.hpp>
#include "bindpoints.hpp"
#include "buffer_vk.hpp"
#include "descriptor_set.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>
#include <memory_resource>
#include <string_view>
#include <vector>

class PipelineVK {
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkPipeline m_pipelineDepthPrepass = VK_NULL_HANDLE;
    Bindpoints m_bindpoints{};
    uint32_t m_pushConstantSize = 0;
    uint32_t m_vertexStride = 0;
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
    ) noexcept;

    PipelineVK(
        const PipelineCreateInfo&
        , VkDevice
        , VkDescriptorSetLayout
    ) noexcept;

    PipelineVK( PipelineVK&& ) noexcept;
    PipelineVK& operator = ( PipelineVK&& ) noexcept;

    operator VkPipeline () const;
    VkPipeline depthPrepass() const;

    VkPipelineLayout layout() const;
    uint32_t pushConstantSize() const;
    uint32_t vertexStride() const;
    Bindpoints bindpoints() const;

    bool depthWrite() const;
    bool useLines() const;
};
