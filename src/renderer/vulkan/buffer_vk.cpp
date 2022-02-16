#include "buffer_vk.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "utils_vk.hpp"

#include <Tracy.hpp>

void BufferVK::destroyResources()
{
    destroy<vkDestroyBuffer>( m_device, m_buffer );
    destroy<vkFreeMemory>( m_device, m_memory );
}

BufferVK::~BufferVK() noexcept
{
    destroyResources();
}

BufferVK::BufferVK( BufferVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_buffer, rhs.m_buffer );
    std::swap( m_size, rhs.m_size );
    std::swap( m_purpose, rhs.m_purpose );
}

BufferVK& BufferVK::operator = ( BufferVK&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_memory = std::exchange( rhs.m_memory, {} );
    m_buffer = std::exchange( rhs.m_buffer, {} );
    m_size = std::exchange( rhs.m_size, {} );
    m_purpose = std::exchange( rhs.m_purpose, Purpose::eStaging );
    return *this;
}

BufferVK::BufferVK( VkPhysicalDevice physicalDevice, VkDevice device, Purpose purpose, uint32_t size ) noexcept
: m_device( device )
, m_size( size )
, m_purpose( purpose )
{
    ZoneScoped;
    VkBufferUsageFlags usage{};
    VkMemoryPropertyFlags flags{};
    switch ( m_purpose ) {
    case Purpose::eStaging:
        usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;

    case Purpose::eVertex:
        usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    }

    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    [[maybe_unused]]
    const VkResult bufferOK = vkCreateBuffer( device, &bufferInfo, nullptr, &m_buffer );
    assert( bufferOK == VK_SUCCESS );

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements( device, m_buffer, &memRequirements );

    const VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryType( physicalDevice, memRequirements.memoryTypeBits, flags ),
    };

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateMemory( device, &allocInfo, nullptr, &m_memory );
    assert( allocOK == VK_SUCCESS );

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
