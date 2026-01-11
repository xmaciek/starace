#pragma once

#include "vk.hpp"

#include <renderer/display_mode.hpp>

#include <array>
#include <memory_resource>
#include <vector>

struct SDL_Window;
class Swapchain {
private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_surfaceFormat{};
    VkExtent2D m_extent{};
    VSync m_vsync{};
    std::pmr::vector<VkImage> m_images{};
    uint32_t m_imageCount = 0;

public:
    void destroyResources();
    ~Swapchain();
    Swapchain() = default;
    Swapchain( SDL_Window*, VkPhysicalDevice, VkDevice, VkSurfaceKHR, std::array<uint32_t, 2> familyAccess, VSync, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE );
    Swapchain( Swapchain&& ) noexcept;
    Swapchain& operator = ( Swapchain&& ) noexcept;

    VkExtent2D extent() const;
    VkSurfaceFormatKHR surfaceFormat() const;
    VkImage image( size_t ) const;
    uint32_t imageCount() const;

    VSync vsync() const;

    [[nodiscard]]
    VkSwapchainKHR steal();

    operator VkSwapchainKHR () const;

    static std::array<bool, 3> supportedVSyncs( VkPhysicalDevice, VkSurfaceKHR );
};
