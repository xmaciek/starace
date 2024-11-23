#include "uniform.hpp"

#include "utils_vk.hpp"

#include <profiler.hpp>

#include <cassert>
#include <cstring>
#include <utility>
#include <bit>


Uniform::~Uniform() noexcept
{
    if ( m_device && m_memoryStaging ) {
        vkUnmapMemory( m_device, m_memoryStaging );
    }
    destroy<vkDestroyBuffer>( m_device, m_buffer );
    destroy<vkDestroyBuffer>( m_device, m_staging );
}

Uniform::Uniform( Uniform&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memoryStaging, rhs.m_memoryStaging );
    std::swap( m_memoryDeviceLocal, rhs.m_memoryDeviceLocal );
    std::swap( m_staging, rhs.m_staging );
    std::swap( m_buffer, rhs.m_buffer );
    std::swap( m_mapped, rhs.m_mapped );
    std::swap( m_currentOffset, rhs.m_currentOffset );
    std::swap( m_minAlign, rhs.m_minAlign );
    std::swap( m_size, rhs.m_size );
}

Uniform& Uniform::operator = ( Uniform&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memoryStaging, rhs.m_memoryStaging );
    std::swap( m_memoryDeviceLocal, rhs.m_memoryDeviceLocal );
    std::swap( m_staging, rhs.m_staging );
    std::swap( m_buffer, rhs.m_buffer );
    std::swap( m_mapped, rhs.m_mapped );
    std::swap( m_currentOffset, rhs.m_currentOffset );
    std::swap( m_minAlign, rhs.m_minAlign );
    std::swap( m_size, rhs.m_size );

    return *this;
}

static VkBuffer createBuffer( VkDevice device, std::size_t size, VkBufferUsageFlags flags ) noexcept
{
    const VkBufferCreateInfo bufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = flags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkBuffer buffer = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult bufferOK = vkCreateBuffer( device, &bufferInfo, nullptr, &buffer );
    assert( bufferOK == VK_SUCCESS );
    return buffer;
}

Uniform::Uniform( VkPhysicalDevice physDevice, VkDevice device, std::size_t size, std::size_t minAlign ) noexcept
: m_device{ device }
, m_minAlign{ minAlign }
, m_size{ size }
{
    ZoneScoped;
    m_staging = createBuffer( device, m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT );
    m_buffer = createBuffer( device, m_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    m_memoryStaging = DeviceMemory{ physDevice, device, m_staging, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    m_memoryDeviceLocal = DeviceMemory{ physDevice, device, m_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    [[maybe_unused]] const VkResult sOK = vkBindBufferMemory( device, m_staging, m_memoryStaging, 0 );
    [[maybe_unused]] const VkResult lOK = vkBindBufferMemory( device, m_buffer, m_memoryDeviceLocal, 0 );
    assert( sOK == VK_SUCCESS );
    assert( lOK == VK_SUCCESS );

    assert( m_size <= m_memoryStaging.size() );
    [[maybe_unused]]
    const VkResult mapOK = vkMapMemory( m_device, m_memoryStaging, 0, m_size, {}, &m_mapped );
    assert( mapOK == VK_SUCCESS );
    reset();
}

static std::uintptr_t align( std::uintptr_t p, std::size_t a )
{
    assert( std::popcount( a ) == 1 );
    const std::size_t bitmask = a - 1;
    return ( p + bitmask ) & ~bitmask;
}

VkDescriptorBufferInfo Uniform::copy( const void* data, std::size_t size ) noexcept
{
    const std::uintptr_t offset = align( m_currentOffset, m_minAlign );
    m_currentOffset = offset + size;
    assert( m_currentOffset <= m_size );
    auto* ptr = reinterpret_cast<std::byte*>( m_mapped ) + offset;
    std::memcpy( ptr, data, size );

    return {
        .buffer = m_buffer,
        .offset = offset,
        .range = size,
    };
}

void Uniform::reset()
{
    m_currentOffset = 0;
}

void Uniform::transfer( VkCommandBuffer cmd )
{
    if ( m_currentOffset == 0 ) {
        return;
    }
    const VkBufferCopy copyRegion{
        .size = m_currentOffset,
    };
    vkCmdCopyBuffer( cmd, m_staging, m_buffer, 1, &copyRegion );
}
