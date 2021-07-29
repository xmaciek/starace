#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <string_view>

class Shader {
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_module = VK_NULL_HANDLE;

public:
    ~Shader() noexcept;
    Shader( VkDevice, std::string_view filePath ) noexcept;

    Shader( const Shader& ) = delete;
    Shader& operator = ( const Shader& ) = delete;
    Shader( Shader&& ) = delete;
    Shader& operator = ( Shader&& ) = delete;

    VkPipelineShaderStageCreateInfo vertex() const;
    VkPipelineShaderStageCreateInfo fragment() const;
    VkPipelineShaderStageCreateInfo compute() const;
};
