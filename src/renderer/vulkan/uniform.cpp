#include "uniform.hpp"

#include "utils_vk.hpp"

#include <cassert>
#include <cstring>

void Uniform::destroyResources() noexcept
{
    if ( m_device && m_memoryStaging ) {
        vkUnmapMemory( m_device, m_memoryStaging );
    }
    destroy<vkDestroyBuffer>( m_device, m_buffer );
    destroy<vkDestroyBuffer>( m_device, m_staging );
    destroy<vkFreeMemory>( m_device, m_memoryDeviceLocal );
    destroy<vkFreeMemory>( m_device, m_memoryStaging );
}

Uniform::~Uniform() noexcept
{
    destroyResources();
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
    std::swap( m_transferSize, rhs.m_transferSize );
    std::swap( m_size, rhs.m_size );
}

Uniform& Uniform::operator = ( Uniform&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_memoryStaging = std::exchange( rhs.m_memoryStaging, {} );
    m_memoryDeviceLocal = std::exchange( rhs.m_memoryDeviceLocal, {} );
    m_staging = std::exchange( rhs.m_staging, {} );
    m_buffer = std::exchange( rhs.m_buffer, {} );
    m_mapped = std::exchange( rhs.m_mapped, {} );
    m_currentOffset = std::exchange( rhs.m_currentOffset, {} );
    m_minAlign = std::exchange( rhs.m_minAlign, {} );
    m_transferSize = std::exchange( rhs.m_transferSize, {} );
    m_size = std::exchange( rhs.m_size, {} );

    return *this;
}


static VkDeviceMemory alloc( VkPhysicalDevice physDevice, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags flags ) noexcept
{
    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements( device, buffer, &memRequirements );
    const VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryType( physDevice, memRequirements.memoryTypeBits, flags ),
    };

    VkDeviceMemory memory = VK_NULL_HANDLE;
    [[maybe_unused]]
    const VkResult allocOK = vkAllocateMemory( device, &allocInfo, nullptr, &memory );
    assert( allocOK == VK_SUCCESS );
    return memory;
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
    m_staging = createBuffer( device, m_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT );
    m_buffer = createBuffer( device, m_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    m_memoryStaging = alloc( physDevice, device, m_staging, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    m_memoryDeviceLocal = alloc( physDevice, device, m_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    [[maybe_unused]] const VkResult sOK = vkBindBufferMemory( device, m_staging, m_memoryStaging, 0 );
    [[maybe_unused]] const VkResult lOK = vkBindBufferMemory( device, m_buffer, m_memoryDeviceLocal, 0 );
    assert( sOK == VK_SUCCESS );
    assert( lOK == VK_SUCCESS );

    [[maybe_unused]]
    const VkResult mapOK = vkMapMemory( m_device, m_memoryStaging, 0, m_size, {}, &m_mapped );
    assert( mapOK == VK_SUCCESS );
    reset();
}

static std::uintptr_t align( std::uintptr_t p, std::size_t a )
{
    assert( std::popcount( a ) == 1 );
    return p + ( -p & ( a - 1 ) );
}

VkDescriptorBufferInfo Uniform::copy( const void* data, std::size_t size ) noexcept
{
    const std::uintptr_t offset = align( m_currentOffset, m_minAlign );
    m_currentOffset = offset + size;
    m_transferSize = offset + size;
    assert( m_transferSize <= m_size );
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
    const std::uintptr_t m = reinterpret_cast<std::uintptr_t>( m_mapped );
    m_currentOffset = align( m, m_minAlign ) - m;
    m_transferSize = 0;
}

void Uniform::transfer( VkCommandBuffer cmd )
{
    if ( m_transferSize == 0 ) {
        return;
    }
    const VkBufferCopy copyRegion{
        .size = m_transferSize,
    };
    vkCmdCopyBuffer( cmd, m_staging, m_buffer, 1, &copyRegion );
}
