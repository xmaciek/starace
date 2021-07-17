#pragma once

#include <vulkan/vulkan.hpp>

#include <array>
#include <memory_resource>
#include <vector>

class Swapchain {
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
    VkSurfaceFormatKHR m_surfaceFormat{};
    VkExtent2D m_extent{};

    std::pmr::vector<VkImage> m_images;
    std::pmr::vector<VkImageView> m_imageViews;

    // TODO: these should be part of render target
    std::pmr::vector<VkImage> m_depth;
    std::pmr::vector<VkImageView> m_depthView;
    std::pmr::vector<VkDeviceMemory> m_depthMemory;
    VkFormat m_depthFormat = {};

    uint32_t m_imageCount = 0;

public:
    void destroyResources();
    ~Swapchain();
    Swapchain() = default;
    Swapchain( VkPhysicalDevice, VkDevice, VkSurfaceKHR, std::array<uint32_t, 2> familyAccess );
    Swapchain( Swapchain&& ) noexcept;
    Swapchain& operator = ( Swapchain&& ) noexcept;

    VkExtent2D extent() const;
    VkSurfaceFormatKHR surfaceFormat() const;
    VkFormat depthFormat() const;
    const std::pmr::vector<VkImageView>& imageViews() const;
    const std::pmr::vector<VkImageView>& depthViews() const;
    uint32_t imageCount() const;

    operator VkSwapchainKHR () const;

};
