#include "buffer_vk.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <numeric>

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
    std::swap( m_subAllocations, rhs.m_subAllocations );
}

BufferVK& BufferVK::operator = ( BufferVK&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_memory, rhs.m_memory );
    std::swap( m_buffer, rhs.m_buffer );
    std::swap( m_subAllocations, rhs.m_subAllocations );
    return *this;
}

BufferVK::BufferVK( VkPhysicalDevice physicalDevice, VkDevice device, const Purpose& purpose, VkDeviceSize size ) noexcept
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
    assert( from.subAllocationSize() <= sizeInBytes() );
    const VkBufferCopy copyRegion{
        .size = from.subAllocationSize(),
    };
    vkCmdCopyBuffer( cmd, from.m_buffer, m_buffer, 1, &copyRegion );
    m_subAllocations = from.m_subAllocations;
    std::ranges::for_each( m_subAllocations, [this]( auto& desc ) { desc.buffer = m_buffer; } );
}

static VkDeviceSize align( VkDeviceSize p, VkDeviceSize a )
{
    assert( std::popcount( a ) == 1 );
    const VkDeviceSize bitmask = a - 1;
    return ( p + bitmask ) & ~bitmask;
}

VkDeviceSize BufferVK::computeSize( std::span<std::span<const uint8_t>> datas, VkDeviceSize minAlignment )
{
    auto comp = [minAlignment]( VkDeviceSize r, const auto& s )
    {
        return align( r + s.size(), minAlignment );
    };
    return std::accumulate( datas.begin(), datas.end(), VkDeviceSize{}, comp );
}

void BufferVK::copyData( std::span<const uint8_t> data )
{
    std::array datas{ data };
    copyDatas( datas, 1 );
}

void BufferVK::copyDatas( std::span<std::span<const uint8_t>> datas, VkDeviceSize minAlignment )
{
    ZoneScoped;
    m_subAllocations.resize( datas.size() );

    void* ptr = nullptr;
    [[maybe_unused]]
    const VkResult mapOK = vkMapMemory( m_device, m_memory, 0, sizeInBytes(), 0, &ptr );
    assert( mapOK == VK_SUCCESS );
    assert( ptr );

    auto copyRange = [this, minAlignment, ptr, offset=VkDeviceSize{}]( std::span<const uint8_t> s ) mutable
    {
        assert( !s.empty() );
        assert( offset + s.size() <= sizeInBytes() );
        std::memcpy( ptr, s.data(), s.size() );
        auto offsetRet = offset;
        offset = align( offset + s.size(), minAlignment );
        ptr = ((uint8_t*)ptr) + align( s.size(), minAlignment );

        return VkDescriptorBufferInfo{
            .buffer = m_buffer,
            .offset = offsetRet,
            .range = s.size(),
        };
    };
    std::ranges::transform( datas, m_subAllocations.begin(), copyRange );
    vkUnmapMemory( m_device, m_memory );
}

void BufferVK::release( const VkDescriptorBufferInfo& buffInfo )
{
    auto it = std::ranges::find_if( m_subAllocations, [&buffInfo]( const auto& it )
    {
        return buffInfo.buffer == it.buffer
            && buffInfo.offset == it.offset
            && buffInfo.range == it.range
        ;
    } );
    if ( it == m_subAllocations.end() ) [[unlikely]] {
        assert( !"BufferVK::release got invalid buffer info" );
        return;
    }
    *it = {};
}

bool BufferVK::empty() const
{
    auto ret = std::accumulate( m_subAllocations.begin(), m_subAllocations.end(), m_subAllocations.size(), []( auto r, const VkDescriptorBufferInfo& bi ) { return r - !bi.buffer; } );
    return ret == 0;
}

std::span<const VkDescriptorBufferInfo> BufferVK::subAllocations() const
{
    return m_subAllocations;
}

BufferVK::operator VkBuffer () const
{
    return m_buffer;
}

VkDeviceSize BufferVK::sizeInBytes() const
{
    return m_memory.size();
}

VkDeviceSize BufferVK::subAllocationSize() const
{
    if ( m_subAllocations.empty() ) return m_memory.size();
    auto&& back = m_subAllocations.back();
    return back.offset + back.range;
}


VkDescriptorBufferInfo BufferVK::bindInfo() const
{
    return {
        .buffer = m_buffer,
        .range = m_memory.size(),
    };
}

void BufferVK::bind( VkCommandBuffer cmd )
{
    VkDeviceSize offset{};
    vkCmdBindVertexBuffers( cmd, 0, 1, &m_buffer, &offset );
}

void BufferVK::bindAsIndex( VkCommandBuffer cmd )
{
    vkCmdBindIndexBuffer( cmd, m_buffer, 0, VK_INDEX_TYPE_UINT16 );
}
