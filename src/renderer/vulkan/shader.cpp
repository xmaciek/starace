#include "shader.hpp"

#include <cassert>

#include "utils_vk.hpp"

Shader::Shader( VkDevice device, std::span<const uint8_t> shaderData ) noexcept
: m_device( device )
{
    assert( shaderData.size() > 0 );
    assert( shaderData.size() % 4 == 0 );
    assert( ( reinterpret_cast<uintptr_t>( shaderData.data() ) & ( sizeof( uint32_t ) - 1u ) ) == 0 ); // data align

    const VkShaderModuleCreateInfo vertexInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderData.size(),
        .pCode = reinterpret_cast<const uint32_t*>( shaderData.data() ),
    };
    [[maybe_unused]]
    const VkResult moduleOK = vkCreateShaderModule( device, &vertexInfo, nullptr, &m_module );
    assert( moduleOK == VK_SUCCESS );
}

Shader::~Shader() noexcept
{
    destroy<vkDestroyShaderModule, VkShaderModule>( m_device, m_module );
}

template <VkShaderStageFlagBits TStage>
static VkPipelineShaderStageCreateInfo getStage( VkShaderModule m )
{
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = TStage,
        .module = m,
        .pName = "main",
    };
}

VkPipelineShaderStageCreateInfo Shader::vertex() const
{
    return getStage<VK_SHADER_STAGE_VERTEX_BIT>( m_module );
}

VkPipelineShaderStageCreateInfo Shader::fragment() const
{
    return getStage<VK_SHADER_STAGE_FRAGMENT_BIT>( m_module );
}

VkPipelineShaderStageCreateInfo Shader::compute() const
{
    return getStage<VK_SHADER_STAGE_COMPUTE_BIT>( m_module );
}
