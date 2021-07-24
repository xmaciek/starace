#include "swapchain.hpp"

#include "utils_vk.hpp"

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
    for ( VkImageView& it : m_depthView ) {
        vkDestroyImageView( m_device, it, nullptr );
    }
    for ( VkImage& it : m_depth ) {
        vkDestroyImage( m_device, it, nullptr );
    }
    for ( VkDeviceMemory& it : m_depthMemory ) {
        vkFreeMemory( m_device, it, nullptr );
    }

    for ( VkImageView& it : m_imageViews ) {
        vkDestroyImageView( m_device, it, nullptr );
    }
    if ( m_swapchain ) {
        vkDestroySwapchainKHR( m_device, m_swapchain, nullptr );
    }
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
    std::swap( m_imageViews, rhs.m_imageViews );
    std::swap( m_depth, rhs.m_depth );
    std::swap( m_depthView, rhs.m_depthView );
    std::swap( m_depthMemory, rhs.m_depthMemory );
    std::swap( m_depthFormat, rhs.m_depthFormat );
}

Swapchain& Swapchain::operator = ( Swapchain&& rhs ) noexcept
{
    destroyResources();
    m_device = rhs.m_device;
    m_swapchain = rhs.m_swapchain;
    m_presentMode = rhs.m_presentMode;
    m_surfaceFormat = rhs.m_surfaceFormat;
    m_extent = rhs.m_extent;
    m_imageCount = rhs.m_imageCount;
    m_images = std::move( rhs.m_images );
    m_imageViews = std::move( rhs.m_imageViews );
    m_depth = std::move( rhs.m_depth );
    m_depthView = std::move( rhs.m_depthView );
    m_depthMemory = std::move( rhs.m_depthMemory );
    m_depthFormat = rhs.m_depthFormat;
    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_swapchain = VK_NULL_HANDLE;
    rhs.m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    rhs.m_surfaceFormat = {};
    rhs.m_imageCount = 0;
    rhs.m_extent= {};
    rhs.m_depthFormat = {};
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

Swapchain::Swapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, std::array<uint32_t,2> familyAccess )
: m_device{ device }
{
    assert( physicalDevice );
    assert( device );

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    [[maybe_unused]]
    const VkResult surfaceOK = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &surfaceCaps );
    assert( surfaceOK == VK_SUCCESS );

    const std::optional<VkSurfaceFormatKHR> surfaceFormat = findBestFormat( physicalDevice, surface );
    assert( surfaceFormat );
    m_surfaceFormat = *surfaceFormat;

    const std::optional<VkPresentModeKHR> presentMode = findBestPresentMode( physicalDevice, surface );
    assert( presentMode );
    m_presentMode = *presentMode;

    m_extent = surfaceCaps.currentExtent;
    m_imageCount = std::clamp( surfaceCaps.minImageCount + 1, surfaceCaps.minImageCount, surfaceCaps.maxImageCount );

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

    m_imageViews.reserve( imageCount );
    for ( const VkImage& it : m_images ) {
        m_imageViews.emplace_back( createImageView( m_device, it, m_surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT ) );
    }

    m_depthFormat = pickSupportedFormat(
        physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    m_depth.reserve( imageCount );
    for ( uint32_t i = 0; i < imageCount; ++i ) {
        auto [ image, memory ] = createImage(
            physicalDevice,
            device,
            m_extent,
            m_depthFormat,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        m_depth.emplace_back( image );
        m_depthMemory.emplace_back( memory );
        m_depthView.emplace_back( createImageView( m_device, image, m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT ) );
    }

}

uint32_t Swapchain::imageCount() const
{
    return m_imageCount;
}

VkSurfaceFormatKHR Swapchain::surfaceFormat() const
{
    return m_surfaceFormat;
}

VkFormat Swapchain::depthFormat() const
{
    return m_depthFormat;
}

VkExtent2D Swapchain::extent() const
{
    return m_extent;
}

const std::pmr::vector<VkImageView>& Swapchain::imageViews() const
{
    return m_imageViews;
}

const std::pmr::vector<VkImageView>& Swapchain::depthViews() const
{
    return m_depthView;
}

Swapchain::operator VkSwapchainKHR () const
{
    return m_swapchain;
}
