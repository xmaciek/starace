#pragma once

#include <renderer/pipeline.hpp>

#include <vulkan/vulkan.h>

#include <string_view>

class PipelineVK {
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSet = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

public:
    ~PipelineVK();
    PipelineVK() = default;

    PipelineVK( VkDevice, VkFormat, const VkExtent2D&, std::string_view vertex, std::string_view fragment );

    void begin( VkCommandBuffer, VkFramebuffer, const VkRect2D& );
    void end( VkCommandBuffer );

};

