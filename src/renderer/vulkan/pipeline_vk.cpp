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
    std::swap( m_pushConstantSize, rhs.m_pushConstantSize );
    std::swap( m_vertexStride, rhs.m_vertexStride );
}

PipelineVK& PipelineVK::operator = ( PipelineVK&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_layout = std::exchange( rhs.m_layout, {} );
    m_pipeline = std::exchange( rhs.m_pipeline, {} );
    m_pushConstantSize = std::exchange( rhs.m_pushConstantSize, {} );
    m_vertexStride = std::exchange( rhs.m_vertexStride, {} );
    return *this;
}

static constexpr auto inputType( PipelineCreateInfo::InputType it )
{
    using enum PipelineCreateInfo::InputType;
    switch ( it ) {
    default:
    case eNone: return VK_FORMAT_UNDEFINED;
    case eF2: return VK_FORMAT_R32G32_SFLOAT;
    case eF3: return VK_FORMAT_R32G32B32_SFLOAT;
    }
}

static constexpr auto attributeAssembly( const PipelineCreateInfo& pci )
{
    constexpr auto maxSize = 3;
    std::array<VkVertexInputAttributeDescription, maxSize> assembly{};
    uint8_t bindCount = 0;
    for ( const auto& it : pci.m_vertexAssembly ) {
        if ( it.m_input == PipelineCreateInfo::InputType::eNone ) { continue; }
        assembly[ bindCount++ ] = VkVertexInputAttributeDescription{
            .location = it.m_location,
            .binding = 0,
            .format = inputType( it.m_input ),
            .offset = it.m_offset,
        };
    }
    return std::make_tuple( assembly, bindCount );
}

static constexpr auto cullMode( PipelineCreateInfo::CullMode cm )
{
    using enum PipelineCreateInfo::CullMode;
    switch ( cm ) {
    default:
    case eNone: return VK_CULL_MODE_NONE;
    case eFront: return VK_CULL_MODE_FRONT_BIT;
    case eBack: return VK_CULL_MODE_BACK_BIT;
    }
}

static constexpr auto frontFace( PipelineCreateInfo::FrontFace ff )
{
    using enum PipelineCreateInfo::FrontFace;
    switch ( ff ) {
    default:
    case eCW: return VK_FRONT_FACE_CLOCKWISE;
    case eCCW: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }
}

static constexpr auto topology( PipelineCreateInfo::Topology tp )
{
    using enum PipelineCreateInfo::Topology;
    switch ( tp ) {
    default: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case eLineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case eLineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case eTriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case eTriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    }
}

PipelineVK::PipelineVK( const PipelineCreateInfo& pci, VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout layout )
: m_device{ device }
, m_pushConstantSize{ pci.m_pushConstantSize }
, m_vertexStride{ pci.m_vertexStride }
{
    assert( device );
    assert( renderPass );
    assert( layout );

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

    static constexpr VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    static VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = cullMode( pci.m_cullMode ),
        .frontFace = frontFace( pci.m_frontFace ),
        .lineWidth = 1.0f,
    };

    const VkPipelineDepthStencilStateCreateInfo depthStencil{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = pci.m_enableDepthTest ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = pci.m_enableDepthWrite ? VK_TRUE : VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    static VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = pci.m_enableBlend ? VK_TRUE : VK_FALSE,
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

    const auto [ attribute, attributeCount ] = attributeAssembly( pci );
    const VkVertexInputBindingDescription vertexBindDescription{
        .binding = 0,
        .stride = pci.m_vertexStride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = attributeCount ? 1u : 0u,
        .pVertexBindingDescriptions = attributeCount ? &vertexBindDescription : nullptr,
        .vertexAttributeDescriptionCount = attributeCount,
        .pVertexAttributeDescriptions = attributeCount ? attribute.data() : nullptr,
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = topology( pci.m_topology ),
        .primitiveRestartEnable = VK_FALSE,
    };

    const Shader vertexShader{ device, pci.m_vertexShader };
    const Shader fragmentShader{ device, pci.m_fragmentShader };
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

PipelineVK::operator VkPipeline () const
{
    return m_pipeline;
}

VkPipelineLayout PipelineVK::layout() const
{
    assert( m_layout );
    return m_layout;
}

uint32_t PipelineVK::pushConstantSize() const
{
    return m_pushConstantSize;
}

uint32_t PipelineVK::vertexStride() const
{
    return m_vertexStride;
}
