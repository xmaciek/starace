#pragma once

#include <vulkan/vulkan.h>

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
    uint32_t m_imageCount = 0;

public:
    void destroyResources();
    ~Swapchain();
    Swapchain() = default;
    Swapchain( VkPhysicalDevice, VkDevice, VkSurfaceKHR, std::array<uint32_t, 2> familyAccess, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
    Swapchain( Swapchain&& ) noexcept;
    Swapchain& operator = ( Swapchain&& ) noexcept;

    VkExtent2D extent() const;
    VkSurfaceFormatKHR surfaceFormat() const;
    VkImage image( size_t ) const;
    uint32_t imageCount() const;

    [[nodiscard]]
    VkSwapchainKHR steal();

    operator VkSwapchainKHR () const;

};
