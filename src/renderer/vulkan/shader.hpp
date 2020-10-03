#pragma once

#include <vulkan/vulkan.h>

#include <array>
#include <string_view>

class Shader {
    VkDevice m_device = VK_NULL_HANDLE;
    VkShaderModule m_moduleVertex = VK_NULL_HANDLE;
    VkShaderModule m_moduleFrgment = VK_NULL_HANDLE;

public:
    ~Shader();
    Shader( VkDevice, std::string_view vertex, std::string_view fragment );

    std::array<VkPipelineShaderStageCreateInfo, 2> stages() const;
};
