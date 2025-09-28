#include "buffer_vk.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "utils_vk.hpp"

#include <profiler.hpp>

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
}

BufferVK& BufferVK::operator = ( BufferVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_buffer, rhs.m_buffer );
    return *this;
}

BufferVK::BufferVK( VkPhysicalDevice physicalDevice, VkDevice device, const Purpose& purpose, uint32_t size ) noexcept
: m_device( device )
{
    ZoneScoped;
    assert( size != 0 );
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
    assert( from.sizeInBytes() == sizeInBytes() );
    const VkBufferCopy copyRegion{
        .size = sizeInBytes(),
    };
    vkCmdCopyBuffer( cmd, from.m_buffer, m_buffer, 1, &copyRegion );
}

void BufferVK::copyData( std::span<const uint8_t> data )
{
    ZoneScoped;
    assert( !data.empty() );
    assert( data.size() <= sizeInBytes() );

    void* ptr = nullptr;
    [[maybe_unused]]
    const VkResult mapOK = vkMapMemory( m_device, m_memory, 0, data.size(), 0, &ptr );
    assert( mapOK == VK_SUCCESS );
    std::memcpy( ptr, data.data(), data.size() );
    vkUnmapMemory( m_device, m_memory );
}

BufferVK::operator VkBuffer () const
{
    return m_buffer;
}

uint32_t BufferVK::sizeInBytes() const
{
    return m_memory.size();
}
