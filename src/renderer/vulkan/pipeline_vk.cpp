#include "pipeline_vk.hpp"

#include <renderer/pipeline.hpp>
#include "shader.hpp"
#include "utils_vk.hpp"

#include <array>
#include <cassert>

void PipelineVK::destroyResources()
{
    destroy<vkDestroyPipeline>( m_device, m_pipeline );
    destroy<vkDestroyPipelineLayout>( m_device, m_layout );
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
    std::swap( m_isActive, rhs.m_isActive );
}

PipelineVK& PipelineVK::operator = ( PipelineVK&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_layout = std::exchange( rhs.m_layout, {} );
    m_pipeline = std::exchange( rhs.m_pipeline, {} );
    m_isActive = std::exchange( rhs.m_isActive, {} );
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

template <> constexpr VkFormat formatForType<float[2]>() noexcept { return VK_FORMAT_R32G32_SFLOAT; }
template <> constexpr VkFormat formatForType<float[3]>() noexcept { return VK_FORMAT_R32G32B32_SFLOAT; }

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
            attribute<float[3], 0, 0>(),
            attribute<float[2], 1, 12>(),
            attribute<float[3], 2, 20>(),
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

PipelineVK::PipelineVK( Pipeline pip, VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout layout, bool depthTest, std::string_view vertex, std::string_view fragment )
: m_device( device )
{
    assert( device );
    assert( renderPass );
    assert( layout );
    assert( !vertex.empty() );
    assert( !fragment.empty() );

    const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &layout,
    };

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

void PipelineVK::end()
{
    m_isActive = false;
}

VkPipelineLayout PipelineVK::layout() const
{
    return m_layout;
}
