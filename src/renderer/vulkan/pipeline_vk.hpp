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
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    DescriptorSet m_descriptorSet;

    bool m_isActive = false;
    void destroyResources();

public:
    ~PipelineVK();
    PipelineVK() = default;

    PipelineVK( VkDevice, VkFormat, uint32_t swapchainCount, const VkExtent2D&, std::string_view vertex, std::string_view fragment );
    PipelineVK( PipelineVK&& ) noexcept;
    PipelineVK& operator = ( PipelineVK&& ) noexcept;

    VkDescriptorSet nextDescriptor();
    void resetDescriptors();
    void begin( VkCommandBuffer, VkFramebuffer, const VkRect2D&, VkDescriptorSet );
    void updateUniforms( const VkBuffer&, uint32_t, VkImageView, VkSampler, VkDescriptorSet );
    void end( VkCommandBuffer );

    VkPipelineLayout layout() const;
};
