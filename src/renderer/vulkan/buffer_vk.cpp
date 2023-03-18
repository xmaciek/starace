#include "buffer_vk.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "utils_vk.hpp"

#include <Tracy.hpp>

BufferVK::~BufferVK() noexcept
{
    ZoneScoped;
    destroy<vkDestroyBuffer>( m_device, m_buffer );
}

BufferVK::BufferVK( BufferVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_buffer, rhs.m_buffer );
    std::swap( m_size, rhs.m_size );
}

BufferVK& BufferVK::operator = ( BufferVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_buffer, rhs.m_buffer );
    std::swap( m_size, rhs.m_size );
    return *this;
}

BufferVK::BufferVK( VkPhysicalDevice physicalDevice, VkDevice device, const Purpose& purpose, uint32_t size ) noexcept
: m_device( device )
, m_size( size )
{
    ZoneScoped;

    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = purpose.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    [[maybe_unused]]
    const VkResult bufferOK = vkCreateBuffer( device, &bufferInfo, nullptr, &m_buffer );
    assert( bufferOK == VK_SUCCESS );

    m_memory = DeviceMemory{ physicalDevice, m_device, m_buffer, purpose.flags };

    [[maybe_unused]]
    const VkResult bindOK = vkBindBufferMemory( device, m_buffer, m_memory, 0 );
    assert( bindOK == VK_SUCCESS );
}

void BufferVK::transferFrom( const BufferVK& from, VkCommandBuffer cmd )
{
    ZoneScoped;
    assert( from.m_size == m_size );
    const VkBufferCopy copyRegion{
        .size = m_size,
    };
    vkCmdCopyBuffer( cmd, from.m_buffer, m_buffer, 1, &copyRegion );
}

void BufferVK::copyData( const uint8_t* data )
{
    ZoneScoped;
    assert( data );
    void* ptr = nullptr;
    assert( m_size <= m_memory.size() );
    [[maybe_unused]]
    const VkResult mapOK = vkMapMemory( m_device, m_memory, 0, m_size, 0, &ptr );
    assert( mapOK == VK_SUCCESS );
    std::memcpy( ptr, data, m_size );
    vkUnmapMemory( m_device, m_memory );
}

BufferVK::operator VkBuffer () const
{
    return m_buffer;
}

uint32_t BufferVK::sizeInBytes() const
{
    return m_size;
}
