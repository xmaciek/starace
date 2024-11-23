#include "swapchain.hpp"

#include "utils_vk.hpp"

#include <profiler.hpp>

#include <algorithm>
#include <cassert>
#include <memory_resource>
#include <optional>
#include <vector>
#include <utility>

constexpr static bool operator == ( const VkSurfaceFormatKHR& lhs, const VkSurfaceFormatKHR& rhs ) noexcept
{
    return lhs.format == rhs.format
        && lhs.colorSpace == rhs.colorSpace;
        ;
}

void Swapchain::destroyResources()
{
    destroy<vkDestroySwapchainKHR, VkSwapchainKHR>( m_device, m_swapchain );
}

Swapchain::~Swapchain()
{
    destroyResources();
}

Swapchain::Swapchain( Swapchain&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_swapchain, rhs.m_swapchain );
    std::swap( m_surfaceFormat, rhs.m_surfaceFormat );
    std::swap( m_extent, rhs.m_extent );
    std::swap( m_vsync, rhs.m_vsync );
    std::swap( m_images, rhs.m_images );
    std::swap( m_imageCount, rhs.m_imageCount );
}

Swapchain& Swapchain::operator = ( Swapchain&& rhs ) noexcept
{
    std::swap( m_device, rhs.m_device );
    std::swap( m_swapchain, rhs.m_swapchain );
    std::swap( m_surfaceFormat, rhs.m_surfaceFormat );
    std::swap( m_extent, rhs.m_extent );
    std::swap( m_vsync, rhs.m_vsync );
    std::swap( m_imageCount, rhs.m_imageCount );
    std::swap( m_images, rhs.m_images );
    return *this;
}


static std::optional<VkSurfaceFormatKHR> findBestFormat( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
{
    assert( physicalDevice );
    assert( surface );

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, nullptr );
    std::pmr::vector<VkSurfaceFormatKHR> formats( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, formats.data() );

    static constexpr VkSurfaceFormatKHR prefferedFormats[] {
        { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
    };
    for ( const VkSurfaceFormatKHR& it : prefferedFormats ) {
        const auto found = std::find( formats.cbegin(), formats.cend(), it );
        if ( found != formats.cend() ) {
            return it;
        }
    }
    return {};
}

static VkPresentModeKHR mapVSync( VSync mode )
{
    switch ( mode ) {
    default:
        assert( !"unhandled enum" );
        [[fallthrough]];
    case VSync::eOff: return VK_PRESENT_MODE_IMMEDIATE_KHR;
    case VSync::eOn: return VK_PRESENT_MODE_FIFO_KHR;
    case VSync::eMailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
    }
}


std::array<bool, 3> Swapchain::supportedVSyncs( VkPhysicalDevice device, VkSurfaceKHR surface )
{
    assert( device );
    assert( surface );

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, nullptr );
    std::pmr::vector<VkPresentModeKHR> presentModes( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, presentModes.data() );

    std::array<bool, 3> ret{};

    for ( const VkPresentModeKHR presentMode : presentModes ) {
        switch ( presentMode ) {
        case VK_PRESENT_MODE_IMMEDIATE_KHR: ret[ (uint32_t)VSync::eOff ] = true; continue;
        case VK_PRESENT_MODE_FIFO_KHR: ret[ (uint32_t)VSync::eOn ] = true; continue;
        case VK_PRESENT_MODE_MAILBOX_KHR: ret[ (uint32_t)VSync::eMailbox ] = true; continue;
        default: continue;
        }
    }
    return ret;
}

static VkPresentModeKHR findBestPresentMode( VkPhysicalDevice device, VkSurfaceKHR surface, VSync v )
{
    assert( device );
    assert( surface );

    const auto vs = Swapchain::supportedVSyncs( device, surface );
    if ( vs[ static_cast<uint32_t>( v ) ] ) {
        return mapVSync( v );
    }

    auto f = std::find( vs.begin(), vs.end(), true );
    assert( f != vs.end() );
    v = static_cast<VSync>( std::distance( vs.begin(), f ) );
    return mapVSync( v );
}

Swapchain::Swapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, std::array<uint32_t,2> familyAccess, VSync vsync, VkSwapchainKHR oldSwapchain )
: m_device{ device }
, m_vsync{ vsync }
{
    ZoneScoped;
    assert( physicalDevice );
    assert( device );

    for ( uint32_t idx : familyAccess ) {
        VkBool32 supportOK = VK_FALSE;
        [[maybe_unused]]
        const VkResult surface0OK = vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, idx, surface, &supportOK );
        assert( supportOK == VK_TRUE );
    }

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    [[maybe_unused]]
    const VkResult surfaceCapsOK = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &surfaceCaps );
    assert( surfaceCapsOK == VK_SUCCESS );

    const std::optional<VkSurfaceFormatKHR> surfaceFormat = findBestFormat( physicalDevice, surface );
    assert( surfaceFormat );
    m_surfaceFormat = *surfaceFormat;

    const VkPresentModeKHR presentMode = findBestPresentMode( physicalDevice, surface, vsync );

    m_extent = surfaceCaps.currentExtent;
    m_imageCount = std::clamp<uint32_t>( 3u, surfaceCaps.minImageCount, surfaceCaps.maxImageCount );

    const VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = m_imageCount,
        .imageFormat = m_surfaceFormat.format,
        .imageColorSpace = m_surfaceFormat.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = familyAccess.size(),
        .pQueueFamilyIndices = familyAccess.data(),
        .preTransform = surfaceCaps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_FALSE,
        .oldSwapchain = oldSwapchain,
    };

    [[maybe_unused]]
    const VkResult swapchainOK = vkCreateSwapchainKHR( m_device, &createInfo, nullptr, &m_swapchain );
    assert( swapchainOK == VK_SUCCESS );

    destroy<vkDestroySwapchainKHR, VkSwapchainKHR>( m_device, oldSwapchain );

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR( m_device, m_swapchain, &imageCount, nullptr );
    assert( imageCount > 0 );
    assert( m_imageCount == imageCount );
    m_images.resize( imageCount );
    [[maybe_unused]]
    const VkResult getImagesOK = vkGetSwapchainImagesKHR( m_device, m_swapchain, &imageCount, m_images.data() );
    assert( getImagesOK == VK_SUCCESS );

}

VkSwapchainKHR Swapchain::steal()
{
    return std::exchange( m_swapchain, VK_NULL_HANDLE );
}

uint32_t Swapchain::imageCount() const
{
    return m_imageCount;
}

VkSurfaceFormatKHR Swapchain::surfaceFormat() const
{
    return m_surfaceFormat;
}

VkExtent2D Swapchain::extent() const
{
    return m_extent;
}

Swapchain::operator VkSwapchainKHR () const
{
    return m_swapchain;
}

VSync Swapchain::vsync() const
{
    return m_vsync;
}

VkImage Swapchain::image( size_t i ) const
{
    assert( i <= m_images.size() );
    return m_images[ i ];
}
