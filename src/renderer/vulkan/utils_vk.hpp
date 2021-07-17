#pragma once

#include <vulkan/vulkan.h>

#include <memory_resource>
#include <vector>
#include <utility>

std::pair<VkImage, VkDeviceMemory> createImage( VkPhysicalDevice, VkDevice, VkExtent2D, VkFormat, VkImageUsageFlags, VkMemoryPropertyFlags );

VkImageView createImageView( VkDevice, VkImage, VkFormat, VkImageAspectFlagBits );

VkFormat pickSupportedFormat( VkPhysicalDevice, const std::pmr::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags );
