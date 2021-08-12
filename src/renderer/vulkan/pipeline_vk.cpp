#include "pipeline_vk.hpp"

#include <renderer/pipeline.hpp>
#include "shader.hpp"
#include "utils_vk.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <cassert>

void PipelineVK::destroyResources()
{
    if ( m_pipeline ) {
        vkDestroyPipeline( m_device, m_pipeline, nullptr );
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
    std::swap( m_pipeline, rhs.m_pipeline );
    std::swap( m_descriptorSet, rhs.m_descriptorSet );
}

PipelineVK& PipelineVK::operator = ( PipelineVK&& rhs ) noexcept
{
    destroyResources();
    m_descriptorSet = std::move( rhs.m_descriptorSet );
    moveClear( m_device, rhs.m_device );
    moveClear( m_layout, rhs.m_layout );
    moveClear( m_pipeline, rhs.m_pipeline );

    return *this;
}

static VkPrimitiveTopology topology( Pipeline pip ) noexcept
{
    switch ( pip ) {
    case Pipeline::eLine3dStripColor:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case Pipeline::eGuiTextureColor1:
    case Pipeline::eTriangleFan3dColor:
    case Pipeline::eTriangleFan3dTexture:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    case Pipeline::eTriangle3dTextureNormal:
    case Pipeline::eShortString:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case Pipeline::eLine3dColor1:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    default:
        assert( !"unhandled enum" );
    }
    return {};
}

template <typename T>
constexpr VkFormat formatForType() noexcept;

template <> constexpr VkFormat formatForType<glm::vec2>() noexcept { return VK_FORMAT_R32G32_SFLOAT; }
template <> constexpr VkFormat formatForType<glm::vec3>() noexcept { return VK_FORMAT_R32G32B32_SFLOAT; }

template <typename T, size_t TLocation, size_t TOffset>
static constexpr VkVertexInputAttributeDescription attribute() noexcept
{
    return {
        .location = TLocation,
        .binding = 0,
        .format = formatForType<T>(),
        .offset = TOffset,
    };
}

static VkPipelineVertexInputStateCreateInfo vertexInfo( Pipeline pip ) noexcept
{
    switch ( pip ) {
    case Pipeline::eGuiTextureColor1:
    case Pipeline::eLine3dColor1:
    case Pipeline::eLine3dStripColor:
    case Pipeline::eTriangleFan3dColor:
    case Pipeline::eTriangleFan3dTexture:
    case Pipeline::eShortString:
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
        };

    case Pipeline::eTriangle3dTextureNormal: {
        static constexpr VkVertexInputBindingDescription bind{
            .binding = 0,
            .stride = sizeof( float ) * 8,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
        static constexpr std::array attr = {
            attribute<glm::vec3, 0, 0>(),
            attribute<glm::vec2, 1, 12>(),
            attribute<glm::vec3, 2, 20>(),
        };
        return {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, \
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bind,
            .vertexAttributeDescriptionCount = attr.size(),
            .pVertexAttributeDescriptions = attr.data(),
        };
    }
    default:
        assert( !"unhandled enum" );
    }
    return {};

}

PipelineVK::PipelineVK( Pipeline pip, VkDevice device, VkRenderPass renderPass, bool depthTest, uint32_t swapchainCount, std::string_view vertex, std::string_view fragment )
: m_device( device )
{
    assert( device );
    assert( renderPass );

    m_descriptorSet = DescriptorSet{ device, swapchainCount, 400,
        {
            std::make_pair( VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT )
            , std::make_pair( VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT )
        }
    };

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = m_descriptorSet.layout(),
    };
    assert( pipelineLayoutInfo.pSetLayouts != VK_NULL_HANDLE );

    [[maybe_unused]]
    const VkResult layoutOK = vkCreatePipelineLayout( m_device, &pipelineLayoutInfo, nullptr, &m_layout );
    assert( layoutOK == VK_SUCCESS );

    static constexpr VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    static constexpr VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0f,
    };

    static constexpr VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = depthTest ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = depthTest ? VK_TRUE : VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    static constexpr VkPipelineColorBlendAttachmentState colorBlendAttachment{
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

    static constexpr std::array dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    const VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStates.size(),
        .pDynamicStates = dynamicStates.data(),
    };

    const VkPipelineVertexInputStateCreateInfo vertexInputInfo = vertexInfo( pip );

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = topology( pip ),
        .primitiveRestartEnable = VK_FALSE,
    };

    const Shader vertexShader{ device, vertex };
    const Shader fragmentShader{ device, fragment };
    const std::array stages = {
        vertexShader.vertex(),
        fragmentShader.fragment()
    };

    const VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = stages.size(),
        .pStages = stages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = m_layout,
        .renderPass = renderPass,
    };

//     assert( viewportState.scissorCount == 1 );
    [[maybe_unused]]
    const VkResult pipelineOK = vkCreateGraphicsPipelines( device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline );
    assert( pipelineOK == VK_SUCCESS );
}

void PipelineVK::begin( VkCommandBuffer cmdBuff, VkDescriptorSet descriptorSet )
{
    if ( !m_isActive ) {
        m_isActive = true;
        vkCmdBindPipeline( cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline );
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
    const uint32_t writeCount = imageView ? (uint32_t)std::size( descriptorWrites ) : 1u;
    vkUpdateDescriptorSets( m_device, writeCount, descriptorWrites, 0, nullptr );
}

VkDescriptorSet PipelineVK::nextDescriptor()
{
    return m_descriptorSet.next();
}

void PipelineVK::resetDescriptors()
{
    m_descriptorSet.reset();
}

void PipelineVK::end()
{
    m_isActive = false;
}

VkPipelineLayout PipelineVK::layout() const
{
    return m_layout;
}
