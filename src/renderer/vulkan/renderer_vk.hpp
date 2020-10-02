#pragma once

#include <renderer/renderer.hpp>

#include <vulkan/vulkan.h>

class RendererVK : public Renderer {
    SDL_Window* m_window = nullptr;

    VkDebugUtilsMessengerEXT m_debug = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    uint32_t m_queueFamilyPresent = 0;
    uint32_t m_queueFamilyGraphics = 0;
    VkQueue m_queuePresent{};
    VkQueue m_queueGraphics{};

    VkDevice m_device = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_swapchainFormat{};
    VkPresentModeKHR m_swapchainMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    uint32_t m_swapchainImageCount = 0;
    std::pmr::vector<VkImage> m_swapchainImages;
    std::pmr::vector<VkImageView> m_swapchainImageViews;

public:
    virtual ~RendererVK() override;
    RendererVK( SDL_Window* );

    virtual Buffer createBuffer( std::pmr::vector<glm::vec2>&&, Buffer::Lifetime ) override;
    virtual Buffer createBuffer( std::pmr::vector<glm::vec3>&&, Buffer::Lifetime ) override;
    virtual Buffer createBuffer( std::pmr::vector<glm::vec4>&&, Buffer::Lifetime ) override;
    virtual std::pmr::memory_resource* allocator() override;
    virtual uint32_t createTexture( uint32_t w, uint32_t h, TextureFormat, bool, const uint8_t* ) override;
    virtual void clear() override;
    virtual void deleteBuffer( const Buffer& ) override;
    virtual void deleteTexture( uint32_t ) override;
    virtual void makeCurrentContext() override;
    virtual void present() override;
    virtual void push( void* buffer, void* constant ) override;
    virtual void setViewportSize( uint32_t w, uint32_t h ) override;
};
