#include "pipeline_vk.hpp"

#include <renderer/pipeline.hpp>
#include "shader.hpp"

#include <cassert>
#include <iostream>

void PipelineVK::destroyResources()
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

}

PipelineVK::~PipelineVK()
{
    destroyResources();
}

PipelineVK::PipelineVK( PipelineVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_layout, rhs.m_layout );
    std::swap( m_renderPass, rhs.m_renderPass );
    std::swap( m_pipeline, rhs.m_pipeline );
    std::swap( m_descriptorSet, rhs.m_descriptorSet );
}

PipelineVK& PipelineVK::operator = ( PipelineVK&& rhs ) noexcept
{
    destroyResources();
    m_device = rhs.m_device;
    m_layout = rhs.m_layout;
    m_renderPass = rhs.m_renderPass;
    m_pipeline = rhs.m_pipeline;
    m_descriptorSet = std::move( rhs.m_descriptorSet );

    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_layout = VK_NULL_HANDLE;
    rhs.m_renderPass = VK_NULL_HANDLE;
    rhs.m_pipeline = VK_NULL_HANDLE;

    return *this;
}


PipelineVK::PipelineVK( VkDevice device, VkFormat format, uint32_t swapchainCount, const VkExtent2D& extent, std::string_view vertex, std::string_view fragment )
: m_device( device )
{
    DescriptorSet descriptorSet( device, swapchainCount, 100,
        {
            std::make_pair( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT )
            , std::make_pair( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT )
        }
    );
    m_descriptorSet = std::move( descriptorSet );

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = m_descriptorSet.layout(),
    };
    assert( pipelineLayoutInfo.pSetLayouts != VK_NULL_HANDLE );
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

    const VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
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
        .dependencyCount = 1,
        .pDependencies = &dependency,
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
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor{
        .extent = extent,
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
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
    };

    const VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
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
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = m_layout,
        .renderPass = m_renderPass,
    };

    assert( viewportState.scissorCount == 1 );
    if ( const VkResult res = vkCreateGraphicsPipelines( device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline );
        res != VK_SUCCESS ) {
        assert( !"failed to create pipeline" );
        std::cout << "failed to create pipeline" << std::endl;
        return;
    }
}

void PipelineVK::begin( VkCommandBuffer cmdBuff, VkFramebuffer framebuffer, const VkRect2D& renderArea, VkDescriptorSet descriptorSet )
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
    if ( !m_isActive ) {
        m_isActive = true;
        vkCmdBindPipeline( cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline );
        vkCmdBeginRenderPass( cmdBuff, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
    }
    vkCmdBindDescriptorSets( cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, 1, &descriptorSet, 0, nullptr );
}

void PipelineVK::updateUniforms( const VkBuffer& buff
    , uint32_t buffSize
    , [[maybe_unused]] VkImageView imageView
    , [[maybe_unused]] VkSampler sampler
    , VkDescriptorSet descriptorSet
)
{
    const VkDescriptorBufferInfo bufferInfo{
        .buffer = buff,
        .offset = 0,
        .range = buffSize,
    };
    const VkDescriptorImageInfo imageInfo{
        .sampler = sampler,
        .imageView = imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    const VkWriteDescriptorSet descriptorWrites[] = {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        },
    };
    vkUpdateDescriptorSets( m_device, std::size( descriptorWrites ), descriptorWrites, 0, nullptr );
}

VkDescriptorSet PipelineVK::nextDescriptor()
{
    return m_descriptorSet.next();
}

void PipelineVK::resetDescriptors()
{
    m_descriptorSet.reset();
}

void PipelineVK::end( VkCommandBuffer cmdBuff )
{
    if ( !m_isActive ) {
        return;
    }
    m_isActive = false;
    vkCmdEndRenderPass( cmdBuff );
}

VkPipelineLayout PipelineVK::layout() const
{
    return m_layout;
}
