#include "pipeline_vk.hpp"

#include "shader.hpp"

#include <cassert>
#include <iostream>

PipelineVK::~PipelineVK()
{
    if ( m_pipeline ) {
        vkDestroyPipeline( m_device, m_pipeline, nullptr );
    }
    if ( m_renderPass ) {
        vkDestroyRenderPass( m_device, m_renderPass, nullptr );
    }
    if ( m_layout ) {
        vkDestroyPipelineLayout( m_device, m_layout, nullptr );
    }
    if ( m_descriptorSet ) {
        vkDestroyDescriptorSetLayout( m_device, m_descriptorSet, nullptr );
    }
}

PipelineVK::PipelineVK( VkDevice device, VkFormat format, const VkExtent2D& extent, std::string_view vertex, std::string_view fragment )
: m_device( device )
{
    const VkDescriptorSetLayoutBinding uniformLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };
    const VkDescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    const VkDescriptorSetLayoutBinding layoutBinding[] = {
        uniformLayoutBinding,
        samplerLayoutBinding,
    };

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = std::size( layoutBinding ),
        .pBindings = layoutBinding,
    };

    if ( const VkResult res = vkCreateDescriptorSetLayout( m_device, &layoutInfo, nullptr, &m_descriptorSet );
        res != VK_SUCCESS ) {
        assert( !"failed to create descriptor set" );
        std::cout << "failed to create descriptor set" << std::endl;
        return;
    }


    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &m_descriptorSet,
    };

    if ( const VkResult res = vkCreatePipelineLayout( m_device, &pipelineLayoutInfo, nullptr, &m_layout );
        res != VK_SUCCESS ) {
        assert( !"failed to create pipeline layout" );
        return;
    }

    const VkAttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef,
    };

    const VkAttachmentDescription colorAttachment{
        .format = format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    if ( const VkResult res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderPass );
        res != VK_SUCCESS ) {
        assert( !"failed to create render pass" );
        std::cout << "failed to create render pass" << std::endl;
        return;
    }

    const VkViewport viewport{
        .width = static_cast<float>( extent.width ),
        .height = static_cast<float>( extent.height ),
        .minDepth = -1.0f,
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor{
        .extent = extent
    };

    const VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_TRUE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    const VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    const VkDynamicState dynamicStates[]{
        VK_DYNAMIC_STATE_VIEWPORT,
    };

    const VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = std::size( dynamicStates ),
        .pDynamicStates = dynamicStates,
    };

    const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
        .primitiveRestartEnable = VK_FALSE,
    };

    const Shader shader( device, vertex, fragment );
    const auto stages = shader.stages();

    const VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = stages.size(),
        .pStages = stages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = m_layout,
        .renderPass = m_renderPass,
    };

    if ( const VkResult res = vkCreateGraphicsPipelines( device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline );
        res != VK_SUCCESS ) {
        assert( !"failed to create pipeline" );
        std::cout << "failed to create pipeline" << std::endl;
        return;
    }

}

void PipelineVK::begin( VkCommandBuffer cmdBuff, VkFramebuffer framebuffer, const VkRect2D& renderArea )
{
    const VkClearValue clearColor{ VkClearColorValue{ { 0.0f, 0.0f, 1.0f, 1.0f } } };
    const VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_renderPass,
        .framebuffer = framebuffer,
        .renderArea = renderArea,
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };
    vkCmdBeginRenderPass( cmdBuff, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void PipelineVK::end( VkCommandBuffer cmdBuff )
{
    vkCmdEndRenderPass( cmdBuff );
}
