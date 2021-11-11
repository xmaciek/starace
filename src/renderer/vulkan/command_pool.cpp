#include "command_pool.hpp"

#include "utils_vk.hpp"

#include <cassert>

CommandPool::~CommandPool() noexcept
{
    destroyResources();
}

void CommandPool::destroyResources() noexcept
{
    destroy<vkDestroyCommandPool>( m_device, m_pool );
    m_buffers.clear();
}

VkCommandPool CommandPool::pool() const noexcept
{
    return m_pool;
}

VkQueue CommandPool::queue() const noexcept
{
    return m_queue;
}

VkCommandBuffer CommandPool::buffer() const noexcept
{
    return m_buffers[ m_currentFrame ];
}

void CommandPool::setFrame( uint32_t frame ) noexcept
{
    m_currentFrame = frame;
}

uint32_t CommandPool::queueIndex() const noexcept
{
    return m_queueIndex;
}

CommandPool::CommandPool( VkDevice device, uint32_t commandBuffersCount, uint32_t queueFamily, uint32_t queueIdx ) noexcept
: m_device{ device }
, m_queueIndex{ queueIdx }
{
    assert( device );
    assert( commandBuffersCount > 0 );

    vkGetDeviceQueue( m_device, queueFamily, queueIdx, &m_queue );

    const VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily,
    };

    [[maybe_unused]]
    const VkResult poolOK = vkCreateCommandPool( m_device, &poolInfo, nullptr, &m_pool );
    assert( poolOK == VK_SUCCESS );

    m_buffers.resize( commandBuffersCount );
    const VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = commandBuffersCount,
    };

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateCommandBuffers( m_device, &allocInfo, m_buffers.data() );
    assert( allocOK == VK_SUCCESS );
}

CommandPool::CommandPool( CommandPool&& rhs ) noexcept
: m_device{ rhs.m_device }
, m_pool{ rhs.m_pool }
, m_queue{ rhs.m_queue }
, m_buffers{ std::move( rhs.m_buffers ) }
, m_queueIndex{ rhs.m_queueIndex }
, m_currentFrame{ rhs.m_currentFrame }
{
    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_pool = VK_NULL_HANDLE;
    rhs.m_queue = VK_NULL_HANDLE;
    rhs.m_queueIndex = 0;
    rhs.m_currentFrame = 0;
}

CommandPool& CommandPool::operator = ( CommandPool&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_pool = std::exchange( rhs.m_pool, {} );
    m_queue = std::exchange( rhs.m_queue, {} );
    m_buffers = std::move( rhs.m_buffers );
    m_queueIndex = std::exchange( rhs.m_queueIndex, {} );
    m_currentFrame = std::exchange( rhs.m_currentFrame, {} );
    return *this;
}


void CommandPool::transferBufferAndWait( VkBuffer src, VkBuffer dst, size_t size ) const
{
    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    const VkBufferCopy copyRegion{
        .size = size,
    };

    VkCommandBuffer cmd = buffer();
    vkBeginCommandBuffer( cmd, &beginInfo );
    vkCmdCopyBuffer( cmd, src, dst, 1, &copyRegion );
    vkEndCommandBuffer( cmd );
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    VkQueue q = queue();
    vkQueueSubmit( q, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( q );
}
