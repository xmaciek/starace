#include "device_memory.hpp"

#include <profiler.hpp>

#include <cassert>
#include <utility>

DeviceMemory::~DeviceMemory() noexcept
{
    ZoneScoped;
    if ( m_memory ) {
        vkFreeMemory( m_device, m_memory, 0 );
    }
}


DeviceMemory::DeviceMemory( DeviceMemory&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_size, rhs.m_size );
}

DeviceMemory& DeviceMemory::operator = ( DeviceMemory&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_size, rhs.m_size );
    return *this;
}

static uint32_t memoryType( VkPhysicalDevice device, uint32_t typeBits, VkMemoryPropertyFlags flags )
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
    return 0;
}

struct GetRequirements {
    VkDevice device{};
    auto operator () ( VkBuffer buffer ) const
    {
        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements( device, buffer, &memRequirements );
        return memRequirements;
    }
    auto operator () ( VkImage image ) const
    {
        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements( device, image, &memRequirements );
        return memRequirements;
    }
};

DeviceMemory::DeviceMemory( VkPhysicalDevice physDevice, VkDevice device, std::variant<VkBuffer, VkImage> buffer, VkMemoryPropertyFlags flags ) noexcept
: m_device{ device }
{
    ZoneScoped;
    assert( device );
    VkMemoryRequirements memRequirements = std::visit( GetRequirements{ device }, buffer );
    m_size = memRequirements.size;

    const VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryType( physDevice, memRequirements.memoryTypeBits, flags ),
    };

    [[maybe_unused]]
    const VkResult allocOK = vkAllocateMemory( device, &allocInfo, nullptr, &m_memory );
    assert( allocOK == VK_SUCCESS );
}

DeviceMemory::operator VkDeviceMemory () const noexcept
{
    return m_memory;
}

uint32_t DeviceMemory::size() const noexcept
{
    return static_cast<uint32_t>( m_size );
}
