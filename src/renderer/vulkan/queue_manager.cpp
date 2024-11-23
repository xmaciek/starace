#include "queue_manager.hpp"

#include <profiler.hpp>

#include <utility>
#include <vector>
#include <memory_resource>
#include <cassert>
#include <algorithm>



static constexpr std::array PRIORITIES{ 1.0f, 1.0f, 1.0f };

bool QueueManager::Family::operator < ( const QueueManager::Family& rhs ) const
{
    return count < rhs.count;
}

QueueManager::~QueueManager() = default;

QueueManager::QueueManager( QueueManager&& rhs )
: m_graphics{ std::exchange( rhs.m_graphics, {} ) }
, m_present{ std::exchange( rhs.m_present, {} ) }
, m_transfer{ std::exchange( rhs.m_transfer, {} ) }
, m_graphicsFamily{ std::exchange( rhs.m_graphicsFamily, {} ) }
, m_presentFamily{ std::exchange( rhs.m_presentFamily, {} ) }
, m_transferFamily{ std::exchange( rhs.m_transferFamily, {} ) }
{
}

QueueManager& QueueManager::operator = ( QueueManager&& rhs )
{
    std::swap( m_graphics, rhs.m_graphics );
    std::swap( m_present, rhs.m_present );
    std::swap( m_transfer, rhs.m_transfer );
    std::swap( m_graphicsFamily, rhs.m_graphicsFamily );
    std::swap( m_presentFamily, rhs.m_presentFamily );
    std::swap( m_transferFamily, rhs.m_transferFamily );
    return *this;
}

static std::tuple<QueueManager::Family, QueueManager::Family, QueueManager::Family> selectFamilies( const auto& graphics, const auto& present, const auto& transfer )
{
    for ( auto&& g : graphics ) {
        for ( auto&& p : present ) {
            if ( p == g ) continue;
            for ( auto&& t : transfer ) {
                if ( t == g ) continue;
                if ( t == p ) continue;
                return { g, p, t };
            }
        }
    }

    for ( auto&& g : graphics ) {
        for ( auto&& t : transfer ) {
            if ( t == g ) continue;
            for ( auto&& p : present ) {
                if ( p == g ) continue;
                if ( p == t ) continue;
                return { g, p, t };
            }
        }
    }

    for ( auto&& g : graphics ) {
        for ( auto&& p : present ) {
            if ( p == g ) continue;
            for ( auto&& t : graphics ) {
                if ( t == g ) continue;
                if ( t == p ) continue;
                return { g, p, t };
            }
        }
    }

    for ( auto&& g : graphics ) {
        for ( auto&& p : present ) {
            if ( p == g ) continue;
            return { g, p, g };
        }
    }

    return { graphics.front(), present.front(), transfer.front() };
}

QueueManager::QueueManager( VkPhysicalDevice device, VkSurfaceKHR surface )
{
    ZoneScoped;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, nullptr );
    std::pmr::vector<VkQueueFamilyProperties> vec( count );
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, vec.data() );

    std::pmr::vector<Family> graphicsCandidate;
    std::pmr::vector<Family> presentCandidate;
    // std::pmr::vector<Family> transferCandidate;

    for ( uint32_t i = 0; i < vec.size(); ++i )
    {
        auto& properties = vec[ i ];
        if ( properties.queueFlags & VK_QUEUE_GRAPHICS_BIT ) graphicsCandidate.emplace_back( i, properties.queueCount );
        // if ( properties.queueFlags & VK_QUEUE_TRANSFER_BIT ) transferCandidate.emplace_back( i, properties.queueCount );
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentSupport );
        if ( presentSupport ) presentCandidate.emplace_back( i, properties.queueCount );
    }

    assert( !graphicsCandidate.empty() );
    assert( !presentCandidate.empty() );
    // assert( !transferCandidate.empty() );
    std::sort( graphicsCandidate.begin(), graphicsCandidate.end() );
    std::sort( presentCandidate.begin(), presentCandidate.end() );
    // std::sort( transferCandidate.begin(), transferCandidate.end() );

    std::tie( m_graphicsFamily, m_presentFamily, m_transferFamily ) = selectFamilies( graphicsCandidate, presentCandidate, graphicsCandidate );
}

void QueueManager::acquire( VkDevice device )
{
    assert( device );
    vkGetDeviceQueue( device, m_graphicsFamily.index, 0u, &std::get<VkQueue>( m_graphics ) );
    vkGetDeviceQueue( device, m_presentFamily.index, 0u, &std::get<VkQueue>( m_present ) );
    std::get<uint32_t>( m_graphics ) = 0;
    std::get<uint32_t>( m_present ) = 1;

    auto& [ transq, transId ] = m_transfer;
    if ( m_graphicsFamily.index != m_transferFamily.index ) {
        vkGetDeviceQueue( device, m_transferFamily.index, 0, &transq );
        transId = 2;
    }
    else if ( m_graphicsFamily.count > 1 ) {
        vkGetDeviceQueue( device, m_graphicsFamily.index, 1, &transq );
        transId = 2;
    }
    else {
        vkGetDeviceQueue( device, m_graphicsFamily.index, 0, &transq );
        transId = 0;
    }

}

std::tuple<std::array<VkDeviceQueueCreateInfo, 3>, uint32_t> QueueManager::createInfo() const
{
    std::tuple<std::array<VkDeviceQueueCreateInfo, 3>, uint32_t> ret;
    auto& arr = std::get<0>( ret );
    arr[ 0 ] = VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_graphicsFamily.index,
        .queueCount = 1u + static_cast<uint32_t>( ( m_graphicsFamily.index == m_transferFamily.index ) && m_graphicsFamily.count > 1 ),
        .pQueuePriorities = PRIORITIES.data(),
    };
    arr[ 1 ] = VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_presentFamily.index,
        .queueCount = 1u,
        .pQueuePriorities = PRIORITIES.data(),
    };
    arr[ 2 ] = VkDeviceQueueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_transferFamily.index,
        .queueCount = 1u,
        .pQueuePriorities = PRIORITIES.data(),
    };
    std::get<1>( ret ) = 2u + static_cast<uint32_t>( m_graphicsFamily.index != m_transferFamily.index );
    return ret;
}

std::tuple<VkQueue, std::mutex*> QueueManager::graphics()
{
    auto [ q, idx ] = m_graphics;
    return { q, &m_mutexes[ idx ] };
}

std::tuple<VkQueue, std::mutex*> QueueManager::present()
{
    auto [ q, idx ] = m_present;
    return { q, &m_mutexes[ idx ] };
}

std::tuple<VkQueue, std::mutex*> QueueManager::transfer()
{
    auto [ q, idx ] = m_transfer;
    return { q, &m_mutexes[ idx ] };
}
