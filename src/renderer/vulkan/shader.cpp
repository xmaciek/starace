#include "shader.hpp"

#include <cassert>
#include <fstream>
#include <memory_resource>
#include <iostream>
#include <vector>

static std::pmr::vector<char> getShaderContent( std::string_view filePath )
{
    std::ifstream ifs( std::string{ filePath }, std::ios::binary | std::ios::ate );
    if ( !ifs.is_open() ) {
        assert( !"failed to open shader file" );
        return {};
    }
    const std::size_t size = ifs.tellg();
    assert( size > 0 );
    assert( size % 4 == 0 );
    std::pmr::vector<char> vec( size );
    ifs.seekg( 0 );
    ifs.read( vec.data(), size );
    return vec;
}

Shader::Shader( VkDevice device, std::string_view vertex, std::string_view fragment )
: m_device( device )
{
    std::pmr::vector<char> contentVertex = getShaderContent( vertex );
    std::pmr::vector<char> contentFragment = getShaderContent( fragment );
    assert( !contentVertex.empty() );
    assert( !contentFragment.empty() );

    const VkShaderModuleCreateInfo vertexInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = contentVertex.size(),
        .pCode = reinterpret_cast<uint32_t*>( contentVertex.data() ),
    };
    if ( const VkResult res = vkCreateShaderModule( device, &vertexInfo, nullptr, &m_moduleVertex );
        res != VK_SUCCESS ) {
        assert( !"failed to create vertex shader module" );
        std::cout << "failed to create vertex shader module" << std::endl;
        return;
    }

    const VkShaderModuleCreateInfo fragmentInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = contentFragment.size(),
        .pCode = reinterpret_cast<uint32_t*>( contentFragment.data() ),
    };
    if ( const VkResult res = vkCreateShaderModule( device, &fragmentInfo, nullptr, &m_moduleFrgment );
        res != VK_SUCCESS ) {
        assert( !"failed to create fragment shader module" );
        std::cout << "failed to create fragment shader module" << std::endl;
        return;
    }
}

Shader::~Shader()
{
    if ( m_moduleFrgment ) {
        vkDestroyShaderModule( m_device, m_moduleFrgment, nullptr );
    }
    if ( m_moduleVertex ) {
        vkDestroyShaderModule( m_device, m_moduleVertex, nullptr );
    }
}

std::array<VkPipelineShaderStageCreateInfo, 2> Shader::stages() const
{
    const VkPipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = m_moduleVertex,
        .pName = "main",
    };

    const VkPipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = m_moduleFrgment,
        .pName = "main",
    };
    return { vertShaderStageInfo, fragShaderStageInfo };
}
