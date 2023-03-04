#pragma once

#include "vk.hpp"

#include <cstdint>
#include <span>

class Shader {
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_module = VK_NULL_HANDLE;

public:
    ~Shader() noexcept;
    Shader( VkDevice, std::span<const uint8_t> shaderData ) noexcept;

    Shader( const Shader& ) = delete;
    Shader& operator = ( const Shader& ) = delete;
    Shader( Shader&& ) = delete;
    Shader& operator = ( Shader&& ) = delete;

    VkPipelineShaderStageCreateInfo vertex() const;
    VkPipelineShaderStageCreateInfo fragment() const;
    VkPipelineShaderStageCreateInfo compute() const;
};
