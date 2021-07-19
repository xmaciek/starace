#include "buffer_pool.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "utils_vk.hpp"

BufferTransfer::BufferTransfer( VkDevice device, VkDeviceMemory mem, VkBuffer s, VkBuffer l, size_t size ) noexcept
: m_device{ device }
, m_stagingMemory{ mem }
, m_staging{ s }
, m_deviceLocal{ l }
, m_size{ size }
{
}

VkBuffer BufferTransfer::dst() const
{
    return m_deviceLocal;
}

VkBuffer BufferTransfer::staging() const
{
    return m_staging;
}

void BufferTransfer::copyToStaging( const uint8_t* data )
{
    assert( data );
    void* ptr = nullptr;
    [[maybe_unused]]
    const VkResult mapOK = vkMapMemory( m_device, m_stagingMemory, 0, m_size, 0, &ptr );
    assert( mapOK == VK_SUCCESS );
    std::memcpy( ptr, data, m_size );
    vkUnmapMemory( m_device, m_stagingMemory );
}

size_t BufferTransfer::sizeInBytes() const
{
    return m_size;
}

void BufferPool::destroyResources()
{
    reset();
    for ( auto& vec : m_available ) {
        for ( BufferTransfer& it : vec.second ) {
            vkDestroyBuffer( m_device, it.staging(), nullptr );
            vkDestroyBuffer( m_device, it.dst(), nullptr );
        }
    }
    for ( VkDeviceMemory it : m_deviceMemory ) {
        vkFreeMemory( m_device, it, nullptr );
    }
}

void BufferPool::reset()
{
    for ( auto& it : m_pending ) {
        std::pmr::vector<BufferTransfer>& vec = m_available[ it.first ];
        const size_t lastSize = vec.size();
        vec.resize( vec.size() + it.second.size() );
        auto vit = vec.begin();
        std::advance( vit, lastSize );
        std::copy_n( it.second.begin(), it.second.size(), vit );
    }
    m_pending.clear();
}

BufferPool::~BufferPool() noexcept
{
    destroyResources();
}

BufferPool::BufferPool( BufferPool&& rhs ) noexcept
{
    std::swap( m_physDevice, rhs.m_physDevice );
    std::swap( m_device, rhs.m_device );
    std::swap( m_available, rhs.m_available );
    std::swap( m_pending, rhs.m_pending );
    std::swap( m_deviceMemory, rhs.m_deviceMemory );
}

BufferPool& BufferPool::operator = ( BufferPool&& rhs ) noexcept
{
    destroyResources();
    m_physDevice = rhs.m_physDevice;
    rhs.m_physDevice = VK_NULL_HANDLE;
    m_device = rhs.m_device;
    rhs.m_device = VK_NULL_HANDLE;
    m_available = std::move( rhs.m_available );
    m_pending = std::move( rhs.m_pending );
    m_deviceMemory = std::move( rhs.m_deviceMemory );
    return *this;
}

BufferPool::BufferPool( VkPhysicalDevice physDevice, VkDevice device ) noexcept
: m_physDevice{ physDevice }
, m_device{ device }
{
}

static VkDeviceMemory alloc( VkPhysicalDevice physDevice, VkDevice device, VkBuffer buffer, VkMemoryPropertyFlags flags )
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

static VkBuffer createBuffer( VkDevice device, size_t size, VkBufferUsageFlags flags )
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

std::tuple<VkBuffer, VkBuffer, VkDeviceMemory, VkDeviceMemory> BufferPool::create( size_t size )
{
    VkBuffer staging = createBuffer( m_device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT );
    VkBuffer deviceLocal = createBuffer( m_device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkDeviceMemory stagingMem = alloc( m_physDevice, m_device, staging, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    VkDeviceMemory deviceLocalMem = alloc( m_physDevice, m_device, deviceLocal, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    [[maybe_unused]] const VkResult sOK = vkBindBufferMemory( m_device, staging, stagingMem, 0 );
    [[maybe_unused]] const VkResult lOK = vkBindBufferMemory( m_device, deviceLocal, deviceLocalMem, 0 );
    assert( sOK == VK_SUCCESS );
    assert( lOK == VK_SUCCESS );

    return { staging, deviceLocal, stagingMem, deviceLocalMem };
}

void BufferPool::reserve( size_t size, size_t count )
{
    std::pmr::vector<BufferTransfer> bvec{};
    std::pmr::vector<VkDeviceMemory> mvec{};
    bvec.reserve( count );
    mvec.reserve( count * 2 );
    m_deviceMemory.reserve( m_deviceMemory.size() + count * 2 );
    for ( size_t i = 0; i < count; ++i ) {
        auto [ sb, lb, sm, lm ] = create( size );
        bvec.emplace_back( m_device, sm, sb, lb, size );
        mvec.emplace_back( sm );
        mvec.emplace_back( lm );
    }

    const size_t mlastSize = m_deviceMemory.size();
    m_deviceMemory.resize( mlastSize + mvec.size() );
    auto mit = m_deviceMemory.begin();
    std::advance( mit, mlastSize );
    std::copy( mvec.begin(), mvec.end(), mit );

    std::pmr::vector<BufferTransfer>& vec = m_available[ size ];
    const size_t blastSize = vec.size();
    vec.resize( blastSize + bvec.size() );
    auto bit = vec.begin();
    std::advance( bit, blastSize );
    std::copy( bvec.begin(), bvec.end(), bit );
}

BufferTransfer BufferPool::getBuffer( size_t sizeInBytes )
{
    std::pmr::vector<BufferTransfer>& bvec = m_available[ sizeInBytes ];
    if ( bvec.empty() ) {
        reserve( sizeInBytes, 10 );
    }
    assert( !bvec.empty() );
    BufferTransfer ret = bvec.back();
    bvec.pop_back();
    m_pending[ sizeInBytes ].emplace_back( ret );
    return ret;
}
