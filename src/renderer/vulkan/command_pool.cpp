#include "command_pool.hpp"

#include <cassert>
#include <iostream>

CommandPool::~CommandPool() noexcept
{
    destroy();
}

void CommandPool::destroy() noexcept
{
    if ( m_pool ) {
        vkDestroyCommandPool( m_device, m_pool, nullptr );
        m_buffers.clear();
    }
}

VkCommandPool CommandPool::pool() const noexcept
{
    return m_pool;
}

VkQueue CommandPool::queue() const noexcept
{
    return m_queues[ m_currentFrame % m_queues.size() ];
}

VkCommandBuffer CommandPool::buffer() const noexcept
{
    return m_buffers[ m_currentFrame ];
}

void CommandPool::setFrame( uint32_t frame ) noexcept
{
    m_currentFrame = frame;
}

CommandPool::CommandPool( VkDevice device, uint32_t commandBuffersCount, std::pair<uint32_t,uint32_t> queues, uint32_t queueFamily ) noexcept
: m_device{ device }
{

    assert( device );
    assert( commandBuffersCount > 0 );
    const auto [ queueCount, queueIndexOffset ] = queues;
    assert( queueCount > 0 );

    m_queues.resize( queueCount );
    for ( uint32_t i = 0; i < queueCount; ++i ) {
        vkGetDeviceQueue( m_device, queueFamily, i + queueIndexOffset, &m_queues[ i ] );
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    [[maybe_unused]]
    const VkResult poolOK = vkCreateCommandPool( m_device, &poolInfo, nullptr, &m_pool );
    assert( poolOK == VK_SUCCESS );

    m_buffers.resize( commandBuffersCount );
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = commandBuffersCount;

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateCommandBuffers( m_device, &allocInfo, m_buffers.data() );
    assert( allocOK == VK_SUCCESS );
}

CommandPool::CommandPool( CommandPool&& rhs ) noexcept
: m_device{ rhs.m_device }
, m_pool{ rhs.m_pool }
, m_queues{ std::move( rhs.m_queues ) }
, m_buffers{ std::move( rhs.m_buffers ) }
, m_currentFrame{ rhs.m_currentFrame }
{
    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_pool = VK_NULL_HANDLE;
    rhs.m_currentFrame = 0;
}

CommandPool& CommandPool::operator = ( CommandPool&& rhs ) noexcept
{
    destroy();
    m_device = rhs.m_device;
    rhs.m_device = VK_NULL_HANDLE;

    m_pool = rhs.m_pool;
    rhs.m_pool = VK_NULL_HANDLE;

    m_queues = std::move( rhs.m_queues );
    m_buffers = std::move( rhs.m_buffers );
    m_currentFrame = rhs.m_currentFrame;
    rhs.m_currentFrame = 0;
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
