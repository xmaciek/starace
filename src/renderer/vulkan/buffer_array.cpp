#include "buffer_array.hpp"

#include <cassert>
#include <iostream>


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

void BufferArray::destroyResources()
{
    for ( const auto& it : m_buffers ) {
        vkDestroyBuffer( m_device, it.first, nullptr );
        vkFreeMemory( m_device, it.second, nullptr );
    }
}

BufferArray::~BufferArray()
{
    destroyResources();
}

BufferArray::BufferArray( BufferArray&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_buffers, rhs.m_buffers );
    std::swap( m_current, rhs.m_current );
    std::swap( m_size, rhs.m_size );
}

BufferArray& BufferArray::operator = ( BufferArray&& rhs ) noexcept
{
    destroyResources();
    m_device = rhs.m_device;
    m_buffers = std::move( rhs.m_buffers );
    m_current = rhs.m_current;
    m_size = rhs.m_size;

    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_current = 0;
    rhs.m_size = 0;

    return *this;
}

BufferArray::BufferArray( VkPhysicalDevice physicalDevice, VkDevice device, uint32_t size, uint32_t count )
: m_device{ device }
, m_buffers( count )
, m_size( size )
{
    const VkBufferUsageFlags usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    const VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    for ( auto& it : m_buffers ) {
         if ( const VkResult res = vkCreateBuffer( device, &bufferInfo, nullptr, &it.first );
            res != VK_SUCCESS ) {
            assert( !"failed to create buffer" );
            std::cout << "failed to create buffer";
            return;
        }

        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements( device, it.first, &memRequirements );
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memType( physicalDevice, memRequirements.memoryTypeBits, flags );
        if ( const VkResult res = vkAllocateMemory( device, &allocInfo, nullptr, &it.second );
            res != VK_SUCCESS) {
            assert( !"failed to allocate gpu memory" );
            std::cout << "failed to allocate gpu memory" << std::endl;
            return;
        }

        if ( const VkResult res = vkBindBufferMemory( device, it.first, it.second, 0 );
            res != VK_SUCCESS ) {
            assert( !"failed to bind buffer to memory" );
            std::cout << "failed to bind buffer to memory" << std::endl;
            return;
        }
    }
}


VkBuffer BufferArray::next()
{
    assert( m_current < m_buffers.size() );
    return m_buffers[ m_current++ ].first;
}

void BufferArray::reset()
{
    m_current = 0;
}

uint32_t BufferArray::size() const
{
    return m_size;
}
