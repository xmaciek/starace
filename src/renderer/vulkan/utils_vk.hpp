#pragma once

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <vector>
#include <utility>
#include <type_traits>

std::tuple<VkImage, VkImageView, VkDeviceMemory> createImage( VkPhysicalDevice, VkDevice, VkExtent2D, VkFormat, VkImageUsageFlags, VkMemoryPropertyFlags, VkImageAspectFlagBits );
VkImageView createImageView( VkDevice, VkImage, VkFormat, VkImageAspectFlagBits );

VkFormat pickSupportedFormat( VkPhysicalDevice, const std::pmr::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags );

uint32_t memoryType( VkPhysicalDevice, uint32_t typeBits, VkMemoryPropertyFlags );

template <typename T>
void moveClear( T& lhs, T& rhs ) noexcept
{
    static_assert( std::is_trivially_copyable_v<T> );
    lhs = rhs;
    rhs = {};
}

template <auto pfnDestroy, typename T>
void destroy( VkDevice device, T t ) noexcept
{
    if ( t ) {
        pfnDestroy( device, t, nullptr );
    }
}
