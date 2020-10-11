#include "single_time_command.hpp"

SingleTimeCommand::~SingleTimeCommand()
{
    vkEndCommandBuffer( m_cmd );

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_cmd,
    };

    vkQueueSubmit( m_queue, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( m_queue );
    vkFreeCommandBuffers( m_device, m_commandPool, 1, &m_cmd );
}

SingleTimeCommand::SingleTimeCommand( VkDevice device, VkCommandPool cmdPool, VkQueue queue )
: m_device{ device }
, m_commandPool{ cmdPool }
, m_queue{ queue }
{
    const VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = cmdPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers( device, &allocInfo, &m_cmd );

    const VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer( m_cmd, &beginInfo );
}

SingleTimeCommand::operator VkCommandBuffer () const
{
    return m_cmd;
}
