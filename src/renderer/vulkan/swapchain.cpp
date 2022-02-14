#include "swapchain.hpp"

#include "utils_vk.hpp"

#include <Tracy.hpp>

#include <cassert>
#include <iostream>
#include <memory_resource>
#include <optional>
#include <vector>

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
    std::swap( m_presentMode, rhs.m_presentMode );
    std::swap( m_surfaceFormat, rhs.m_surfaceFormat );
    std::swap( m_extent, rhs.m_extent );
    std::swap( m_imageCount, rhs.m_imageCount );
    std::swap( m_images, rhs.m_images );
}

Swapchain& Swapchain::operator = ( Swapchain&& rhs ) noexcept
{
    destroyResources();
    m_device = std::exchange( rhs.m_device, {} );
    m_swapchain = std::exchange( rhs.m_swapchain, {} );
    m_presentMode = std::exchange( rhs.m_presentMode, {} );
    m_surfaceFormat = std::exchange( rhs.m_surfaceFormat, {} );
    m_extent = std::exchange( rhs.m_extent, {} );
    m_imageCount = std::exchange( rhs.m_imageCount, {} );
    m_images = std::move( rhs.m_images );
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

static std::optional<VkPresentModeKHR> findBestPresentMode( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface )
{
    assert( physicalDevice );
    assert( surface );

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, nullptr );
    std::pmr::vector<VkPresentModeKHR> presentModes( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, presentModes.data() );

    static constexpr VkPresentModeKHR prefferedPresents[]{
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
    };
    for ( const VkPresentModeKHR& it : prefferedPresents ) {
        const auto found = std::find( presentModes.cbegin(), presentModes.cend(), it );
        if ( found != presentModes.cend() ) {
            return it;
        }
    }
    return {};
}

Swapchain::Swapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, std::array<uint32_t,2> familyAccess, VkSwapchainKHR oldSwapchain )
: m_device{ device }
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

    const std::optional<VkPresentModeKHR> presentMode = findBestPresentMode( physicalDevice, surface );
    assert( presentMode );
    m_presentMode = *presentMode;

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
        .presentMode = m_presentMode,
        .clipped = VK_FALSE,
        .oldSwapchain = oldSwapchain,
    };

    [[maybe_unused]]
    const VkResult swapchainOK = vkCreateSwapchainKHR( m_device, &createInfo, nullptr, &m_swapchain );
    assert( swapchainOK == VK_SUCCESS );

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
    VkSwapchainKHR ret = m_swapchain;
    m_swapchain = VK_NULL_HANDLE;
    return ret;
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

VkImage Swapchain::image( size_t i ) const
{
    assert( i <= m_images.size() );
    return m_images[ i ];
}
