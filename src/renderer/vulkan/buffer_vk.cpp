#include "buffer_vk.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

void BufferVK::destroyResources()
{
    if ( m_buffer ) {
        vkDestroyBuffer( m_device, m_buffer, nullptr );
    }
    if ( m_memory ) {
        vkFreeMemory( m_device, m_memory, nullptr );
    }
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
    m_device = rhs.m_device;
    m_memory = rhs.m_memory;
    m_buffer = rhs.m_buffer;
    m_size = rhs.m_size;
    m_purpose = rhs.m_purpose;

    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_memory = VK_NULL_HANDLE;
    rhs.m_buffer = VK_NULL_HANDLE;
    rhs.m_size = 0;
    rhs.m_purpose = Purpose::eStaging;
    return *this;
}
static uint32_t memType( VkPhysicalDevice device, uint32_t typeBits, VkMemoryPropertyFlags flags )
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties( device, &memProperties );

    for ( uint32_t i = 0; i < memProperties.memoryTypeCount; ++i ) {
        if ( ( typeBits & ( 1 << i ) ) == 0 ) {
            continue;
        }
        if ( ( memProperties.memoryTypes[ i ].propertyFlags & flags ) != flags ) {
            continue;
        }
        return i;
    }
    assert( !"failed to find requested memory type" );
    std::cout << "failedto find requested memory type" << std::endl;
    return 0;
}

BufferVK::BufferVK( VkPhysicalDevice physicalDevice, VkDevice device, Purpose purpose, std::size_t size ) noexcept
: m_device( device )
, m_size( size )
, m_purpose( purpose )
{
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

    case Purpose::eUniform:
        usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    }

    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    if ( const VkResult res = vkCreateBuffer( device, &bufferInfo, nullptr, &m_buffer );
        res != VK_SUCCESS ) {
        assert( !"failed to create buffer" );
        std::cout << "failed to create buffer";
        return;
    }

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements( device, m_buffer, &memRequirements );

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memType( physicalDevice, memRequirements.memoryTypeBits, flags );

    if ( const VkResult res = vkAllocateMemory( device, &allocInfo, nullptr, &m_memory );
        res != VK_SUCCESS) {
        assert( !"failed to allocate gpu memory" );
        std::cout << "failed to allocate gpu memory" << std::endl;
        return;
    }

    if ( const VkResult res = vkBindBufferMemory( device, m_buffer, m_memory, 0 );
        res != VK_SUCCESS ) {
        assert( !"failed to bind buffer to memory" );
        std::cout << "failed to bind buffer to memory" << std::endl;
        return;
    }
}

void BufferVK::transferFrom( const BufferVK& from, VkCommandBuffer cmd )
{
    assert( from.m_size == m_size );
    const VkBufferCopy copyRegion{
        .size = m_size,
    };
    vkCmdCopyBuffer( cmd, from.m_buffer, m_buffer, 1, &copyRegion );
}

void BufferVK::copyData( const uint8_t* data )
{
    assert( data );
    void* ptr = nullptr;
    if ( const VkResult res = vkMapMemory( m_device, m_memory, 0, m_size, 0, &ptr );
        res != VK_SUCCESS ) {
        assert( !"failed to map memory" );
        std::cout << "failed to map memory" << std::endl;
        return;
    }
    std::memcpy( ptr, data, m_size );
    vkUnmapMemory( m_device, m_memory );
}

BufferVK::operator VkBuffer () const
{
    return m_buffer;
}

std::size_t BufferVK::size() const
{
    return m_size;
}
