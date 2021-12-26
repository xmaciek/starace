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
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    bool m_isActive = false;
    void destroyResources();

public:
    ~PipelineVK();
    PipelineVK() = default;

    PipelineVK( const PipelineCreateInfo&, VkDevice, VkRenderPass, VkDescriptorSetLayout );

    PipelineVK( PipelineVK&& ) noexcept;
    PipelineVK& operator = ( PipelineVK&& ) noexcept;

    void begin( VkCommandBuffer, VkDescriptorSet );
    void end();

    VkPipelineLayout layout() const;
};
