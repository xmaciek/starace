#include "renderer_vk.hpp"

#include <SDL2/SDL_vulkan.h>

#include <cassert>
#include <iostream>

Renderer* Renderer::s_instance = nullptr;
static PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT{};
static PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT{};

static std::pmr::vector<const char*> extensions( SDL_Window* window )
{
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions( window, &count, nullptr );
    std::pmr::vector<const char*> ext( count );
    SDL_Vulkan_GetInstanceExtensions( window, &count, ext.data() );
    ext.emplace_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    return ext;
}

static std::pmr::vector<const char*> layers()
{
    return {
        "VK_LAYER_KHRONOS_validation"
    };
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity
    , VkDebugUtilsMessageTypeFlagsEXT type
    , const VkDebugUtilsMessengerCallbackDataEXT* data
    , void* )
{
    static constexpr uint32_t expectedSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    if ( !( severity & expectedSeverity ) ) {
        return VK_FALSE;
    }

    static constexpr uint32_t expectedType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    if ( !( type & expectedType ) ) {
        return VK_FALSE;
    }

    std::cout << "[ FAIL ] " << data->pMessage << std::endl;
    return VK_FALSE;
}

static std::pmr::vector<VkPhysicalDevice> devices( VkInstance instance )
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices( instance, &count, nullptr );
    std::pmr::vector<VkPhysicalDevice> dev( count );
    vkEnumeratePhysicalDevices( instance, &count, dev.data() );
    return dev;
}

static bool testBit( uint32_t a, uint32_t bit, uint32_t bitnot )
{
    return ( a & bit ) == bit && ( a & bitnot ) == 0;
}

uint32_t queueFamilyGraphics( VkPhysicalDevice device )
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, nullptr );
    std::vector<VkQueueFamilyProperties> vec( count );
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, vec.data() );
    for ( uint32_t i = 0; i < vec.size(); ++i ) {
        if ( testBit( vec[ i ].queueFlags, VK_QUEUE_GRAPHICS_BIT, 0 ) ) {
            return i;
        }
    }
    return 0;
}


Renderer* Renderer::instance()
{
    return s_instance;
}

SDL_WindowFlags Renderer::windowFlag()
{
    return SDL_WINDOW_VULKAN;
}

Renderer* Renderer::create( SDL_Window* window )
{
    return new RendererVK( window );
}

RendererVK::RendererVK( SDL_Window* window )
: m_window( window )
{
    s_instance = this;

    const std::pmr::vector<const char*> lay = layers();

    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Starace";
        appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        const std::pmr::vector<const char*> ext = extensions( m_window );
        instanceCreateInfo.enabledExtensionCount = ext.size();
        instanceCreateInfo.ppEnabledExtensionNames = ext.data();

        const std::pmr::vector<const char*> lay = layers();
        instanceCreateInfo.enabledLayerCount = lay.size();
        instanceCreateInfo.ppEnabledLayerNames = lay.data();

        const VkResult res = vkCreateInstance( &instanceCreateInfo, nullptr, &m_instance );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "Failed to create instance" << std::endl;
            return;
        }
    }
    {
        VkDebugUtilsMessengerCreateInfoEXT debugMsg{};
        debugMsg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMsg.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMsg.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMsg.pfnUserCallback = debugCallback;

        createDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>( vkGetInstanceProcAddr( m_instance, "vkCreateDebugUtilsMessengerEXT" ) );
        destroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>( vkGetInstanceProcAddr( m_instance, "vkDestroyDebugUtilsMessengerEXT" ) );
        if ( createDebugUtilsMessengerEXT ) {
            const VkResult res = createDebugUtilsMessengerEXT( m_instance, &debugMsg, nullptr, &m_debug );
            assert( res == VK_SUCCESS );
            if ( res != VK_SUCCESS ) {
                std::cout << "Failed to create debug callback" << std::endl;
                return;
            }
        }
    }

    for ( VkPhysicalDevice it : devices( m_instance ) ) {
        VkPhysicalDeviceProperties deviceProperties{};
        vkGetPhysicalDeviceProperties( it, &deviceProperties );
        if ( deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
            continue;
        }
        m_physicalDevice = it;
        break;
    }

    assert( m_physicalDevice );
    m_queueFamilyGraphics = queueFamilyGraphics( m_physicalDevice );

    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = m_queueFamilyGraphics;
        queueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledLayerCount = static_cast<uint32_t>( lay.size() );
        createInfo.ppEnabledLayerNames = lay.data();
        const VkResult res = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "Failed to create device" << std::endl;
            return;
        }
    }

    vkGetDeviceQueue( m_device, m_queueFamilyGraphics, 0, &m_queueGraphics );
}

RendererVK::~RendererVK()
{
    if ( m_device ) {
        vkDestroyDevice( m_device, nullptr );
    }
    if ( m_debug ) {
        destroyDebugUtilsMessengerEXT( m_instance, m_debug, nullptr );
    }
    if ( m_instance ) {
        vkDestroyInstance( m_instance, nullptr );
    }
}

Buffer RendererVK::createBuffer( std::pmr::vector<glm::vec2>&&, Buffer::Lifetime )
{
    return {};
}

Buffer RendererVK::createBuffer( std::pmr::vector<glm::vec3>&&, Buffer::Lifetime )
{
    return {};
}

Buffer RendererVK::createBuffer( std::pmr::vector<glm::vec4>&&, Buffer::Lifetime )
{
    return {};
}

std::pmr::memory_resource* RendererVK::allocator()
{
    return std::pmr::get_default_resource();
}

uint32_t RendererVK::createTexture( uint32_t, uint32_t, TextureFormat, bool, const uint8_t* )
{
    return {};
}

void RendererVK::clear() {}
void RendererVK::deleteBuffer( const Buffer& ) {}
void RendererVK::deleteTexture( uint32_t ) {}
void RendererVK::makeCurrentContext() {}
void RendererVK::present(){}
void RendererVK::push( void*, void* ) {}
void RendererVK::setViewportSize( uint32_t, uint32_t ) {}
