#include "command_pool.hpp"

#include "utils_vk.hpp"

#include <Tracy.hpp>

#include <cassert>
#include <utility>

CommandPool::~CommandPool() noexcept
{
    destroy<vkDestroyCommandPool>( m_device, m_pool );
}

CommandPool::CommandPool( VkDevice device, uint32_t count, uint32_t queueFamily ) noexcept
: m_device{ device }
{
    ZoneScoped;
    assert( device );
    assert( count > 0 );
    assert( count <= m_buffers.size() );

    const VkCommandPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily,
    };

    [[maybe_unused]]
    const VkResult poolOK = vkCreateCommandPool( m_device, &poolInfo, nullptr, &m_pool );
    assert( poolOK == VK_SUCCESS );

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
}

CommandPool& CommandPool::operator = ( CommandPool&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_pool, rhs.m_pool );
    std::swap( m_buffers, rhs.m_buffers );
    return *this;
}

VkCommandBuffer CommandPool::operator [] ( uint32_t idx )
{
    assert( idx < m_buffers.size() );
    return m_buffers[ idx ];
}

void CommandPool::reset()
{
    [[maybe_unused]]
    const VkResult result = vkResetCommandPool( m_device, m_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT );
    assert( result == VK_SUCCESS );
}
