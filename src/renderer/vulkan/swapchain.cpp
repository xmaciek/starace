#include "swapchain.hpp"

#include <cassert>
#include <memory_resource>
#include <vector>

constexpr static bool operator == ( const VkSurfaceFormatKHR& lhs, const VkSurfaceFormatKHR& rhs ) noexcept
{
    return lhs.format == rhs.format
        && lhs.colorSpace == rhs.colorSpace;
        ;
}
constexpr static bool operator != ( const VkSurfaceFormatKHR& lhs, const VkSurfaceFormatKHR& rhs ) noexcept
{
    return lhs.format != rhs.format
        || lhs.colorSpace != rhs.colorSpace;
        ;
}

void Swapchain::destroyResources()
{
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
    rhs.m_device = VK_NULL_HANDLE;
    rhs.m_swapchain = VK_NULL_HANDLE;
    rhs.m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    rhs.m_surfaceFormat = {};
    rhs.m_imageCount = 0;
    rhs.m_extent= {};
    return *this;
}




Swapchain::Swapchain( VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, std::array<uint32_t,2> familyAccess )
: m_device{ device }
{
    VkSurfaceCapabilitiesKHR surfaceCaps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &surfaceCaps );
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, nullptr );
    std::pmr::vector<VkSurfaceFormatKHR> formats( formatCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, formats.data() );

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, nullptr );
    std::pmr::vector<VkPresentModeKHR> presentModes( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, presentModes.data() );

    static constexpr VkSurfaceFormatKHR prefferedFormats[]{
        { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
    };
    for ( const VkSurfaceFormatKHR& it : prefferedFormats ) {
        const auto found = std::find( formats.cbegin(), formats.cend(), it );
        if ( found != formats.cend() ) {
            m_surfaceFormat = it;
            break;
        }
    }
    assert( m_surfaceFormat != VkSurfaceFormatKHR{} );

    static constexpr VkPresentModeKHR prefferedPresents[]{
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR,
    };
    for ( const VkPresentModeKHR& it : prefferedPresents ) {
        const auto found = std::find( presentModes.cbegin(), presentModes.cend(), it );
        if ( found != presentModes.cend() ) {
            m_presentMode = it;
            break;
        }
    }

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

    if ( const VkResult res = vkCreateSwapchainKHR( m_device, &createInfo, nullptr, &m_swapchain );
        res != VK_SUCCESS ) {
        assert( !"failed to create swapchain" );
        return;
    }

    uint32_t imageConut = 0;
    vkGetSwapchainImagesKHR( m_device, m_swapchain, &imageConut, nullptr );
    assert( imageConut > 0 );
    assert( m_imageCount == imageConut );
    m_images.resize( imageConut );
    if ( const VkResult res = vkGetSwapchainImagesKHR( m_device, m_swapchain, &imageConut, m_images.data() );
        res != VK_SUCCESS ) {
        assert( !"failed to get swapchain images" );
        return;
    }
    m_imageViews.reserve( imageConut );
    for ( const VkImage& it : m_images ) {
        assert( it != VK_NULL_HANDLE );
        static constexpr VkComponentMapping components{
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
        };
        static constexpr VkImageSubresourceRange subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
        const VkImageViewCreateInfo imageViewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = it,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_surfaceFormat.format,
            .components = components,
            .subresourceRange = subresourceRange,
        };
        VkImageView view = VK_NULL_HANDLE;
        const VkResult res = vkCreateImageView( m_device, &imageViewInfo, nullptr, &view );
        if ( res != VK_SUCCESS ) {
            assert( !"failed to create image view" );
            return;
        }
        assert( view != VK_NULL_HANDLE );
        m_imageViews.emplace_back( view );
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

VkExtent2D Swapchain::extent() const
{
    return m_extent;
}

const std::pmr::vector<VkImageView>& Swapchain::imageViews() const
{
    return m_imageViews;
}

Swapchain::operator VkSwapchainKHR () const
{
    return m_swapchain;
}
