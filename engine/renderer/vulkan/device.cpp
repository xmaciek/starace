#include "device.hpp"

#include "wishlist.hpp"

#include <profiler.hpp>
#include <platform/utils.hpp>

#include <cassert>
#include <vector>
#include <memory_resource>

Device::~Device()
{
    if ( m_device ) vkDestroyDevice( m_device, nullptr );
}

Device::Device( Device&& rhs )
{
    std::swap( rhs.m_device, m_device );
    std::swap( rhs.m_features, m_features );
}

Device& Device::operator = ( Device&& rhs )
{
    std::swap( rhs.m_device, m_device );
    std::swap( rhs.m_features, m_features );
    return *this;
}

static constexpr VkPhysicalDeviceFeatures DEVICE_FEATURES{
    .wideLines = VK_TRUE,
    .samplerAnisotropy = VK_TRUE,
};

struct ExtraFeatures {
    void* first = nullptr;
    void** pNext = nullptr;
    inline void add( auto& data )
    {
        if ( !first ) first = &data;
        if ( pNext ) *pNext = &data;
        pNext = &data.pNext;
    }
    inline void* list() { return first; }
};

Device::Device( VkPhysicalDevice phys, std::span<const char*> layers, std::span<const VkDeviceQueueCreateInfo> queues )
{
    ZoneScoped;
    assert( phys );
    assert( vkEnumerateDeviceExtensionProperties );
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties( phys, nullptr, &count, nullptr );
    std::pmr::vector<VkExtensionProperties> deviceExtensions( count );
    vkEnumerateDeviceExtensionProperties( phys, nullptr, &count, deviceExtensions.data() );
    std::pmr::vector<const char*> enableExtensions{};
    Wishlist wishlist{ &deviceExtensions, &enableExtensions };
    auto required = [&wishlist]( auto ext )
    {
        if ( wishlist( ext ) ) return;
        platform::showFatalError( "Required GPU extension is missing", ext );
    };
    required( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    required( VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );

    ExtraFeatures extraFeatures{};
    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .dynamicRendering = VK_TRUE,
    };
    extraFeatures.add( dynamicRendering );

    VkPhysicalDeviceFragmentShadingRateFeaturesKHR vrs{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR,
        .pipelineFragmentShadingRate = VK_TRUE,
        .primitiveFragmentShadingRate = VK_TRUE,
        .attachmentFragmentShadingRate = VK_TRUE,
    };
    if ( wishlist( VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME ) ) {
        m_features[ Feature::eVRS ] = true;
        extraFeatures.add( vrs );
    };

    VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures demoteHelper{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES,
        .shaderDemoteToHelperInvocation = VK_TRUE,
    };
    extraFeatures.add( demoteHelper );

    const VkDeviceCreateInfo deviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = extraFeatures.list(),
        .queueCreateInfoCount = static_cast<uint32_t>( queues.size() ),
        .pQueueCreateInfos = queues.data(),
        .enabledLayerCount = static_cast<uint32_t>( layers.size() ),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>( enableExtensions.size() ),
        .ppEnabledExtensionNames = enableExtensions.data(),
        .pEnabledFeatures = &DEVICE_FEATURES,
    };
    [[maybe_unused]]
    const VkResult deviceOK = vkCreateDevice( phys, &deviceCreateInfo, nullptr, &m_device );
    assert( deviceOK == VK_SUCCESS );
}

bool Device::hasFeature( Feature f ) const
{
    return m_features[ f ];
}
