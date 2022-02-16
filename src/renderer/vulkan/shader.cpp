#include "shader.hpp"

#include <cassert>
#include <fstream>
#include <memory_resource>
#include <iostream>
#include <vector>

#include "utils_vk.hpp"

static std::pmr::vector<char> getShaderContent( std::string_view filePath )
{
    std::ifstream ifs( std::string{ filePath }, std::ios::binary | std::ios::ate );
    assert( ifs.is_open() );
    const std::streamsize size = ifs.tellg();
    assert( size > 0 );
    assert( size % 4 == 0 );
    std::pmr::vector<char> vec( static_cast<std::size_t>( size ) );
    ifs.seekg( 0 );
    ifs.read( vec.data(), (int)size );
    return vec;
}

Shader::Shader( VkDevice device, std::string_view filePath ) noexcept
: m_device( device )
{
    std::pmr::vector<char> content = getShaderContent( filePath );
    assert( !content.empty() );

    const VkShaderModuleCreateInfo vertexInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = content.size(),
        .pCode = reinterpret_cast<uint32_t*>( content.data() ),
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
