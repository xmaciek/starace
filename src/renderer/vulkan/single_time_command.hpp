#pragma once

#include <vulkan/vulkan.h>

class SingleTimeCommand {
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandBuffer m_cmd = VK_NULL_HANDLE;
    VkQueue m_queue = VK_NULL_HANDLE;

public:
    ~SingleTimeCommand();
    SingleTimeCommand( VkDevice, VkCommandPool, VkQueue );

    operator VkCommandBuffer () const;
};
