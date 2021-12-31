#include "command_pool.hpp"

#include "utils_vk.hpp"

#include <cassert>

CommandPool::~CommandPool() noexcept
{
    destroyResources();
}

void CommandPool::destroyResources() noexcept
{
    m_buffers.clear();
    destroy<vkDestroyCommandPool>( m_device, m_pool );
}

VkCommandPool CommandPool::pool() const noexcept
{
    return m_pool;
}

VkCommandBuffer CommandPool::buffer() const noexcept
{
    return m_buffers[ m_currentFrame ];
}

void CommandPool::setFrame( uint32_t frame ) noexcept
{
    m_currentFrame = frame;
}

CommandPool::CommandPool( VkDevice device, uint32_t count, uint32_t queueFamily ) noexcept
: m_device{ device }
{
    assert( device );
    assert( count > 0 );


    const VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily,
    };

    [[maybe_unused]]
    const VkResult poolOK = vkCreateCommandPool( m_device, &poolInfo, nullptr, &m_pool );
    assert( poolOK == VK_SUCCESS );

    m_buffers.resize( count );
    const VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count,
    };

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateCommandBuffers( m_device, &allocInfo, m_buffers.data() );
    assert( allocOK == VK_SUCCESS );
}

CommandPool::CommandPool( CommandPool&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_pool, rhs.m_pool );
    std::swap( m_buffers, rhs.m_buffers );
    std::swap( m_currentFrame, rhs.m_currentFrame );
}

CommandPool& CommandPool::operator = ( CommandPool&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_pool = std::exchange( rhs.m_pool, {} );
    m_buffers = std::move( rhs.m_buffers );
    m_currentFrame = std::exchange( rhs.m_currentFrame, {} );
    return *this;
}


void CommandPool::transferBufferAndWait( VkQueue q, VkBuffer src, VkBuffer dst, size_t size ) const
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

    vkQueueSubmit( q, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( q );
}
