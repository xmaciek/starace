#include "renderer_vk.hpp"

#include <SDL2/SDL_vulkan.h>

#include <algorithm>
#include <cassert>
#include <iostream>

Renderer* Renderer::s_instance = nullptr;
static PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT{};
static PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT{};

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

static std::pmr::vector<const char*> extensions( SDL_Window* window )
{
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions( window, &count, nullptr );
    std::pmr::vector<const char*> ext( count );
    SDL_Vulkan_GetInstanceExtensions( window, &count, ext.data() );
    ext.emplace_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    return ext;
}

static std::pmr::vector<const char*> deviceExtensions()
{
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
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

void queueFamilies( VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t* graphics, uint32_t* present, uint32_t* transfer )
{
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, nullptr );
    std::vector<VkQueueFamilyProperties> vec( count );
    std::vector<uint32_t> graphicsCandidate;
    std::vector<uint32_t> presentCandidate;
    std::vector<uint32_t> transferCandidate;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, vec.data() );
    for ( uint32_t i = 0; i < vec.size(); ++i ) {
        if ( testBit( vec[ i ].queueFlags, VK_QUEUE_GRAPHICS_BIT, 0 ) ) {
            graphicsCandidate.emplace_back( i );
        }
        if ( testBit( vec[ i ].queueFlags, VK_QUEUE_TRANSFER_BIT, 0 ) ) {
            transferCandidate.emplace_back( i );
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentSupport );
        if ( presentSupport ) {
            presentCandidate.emplace_back( i );
        }
    }
#if 0
    std::cout << "graphics ";
    for ( uint32_t it : graphicsCandidate ) std::cout << it << " ";
    std::cout << std::endl;

    std::cout << "present ";
    for ( uint32_t it : presentCandidate ) std::cout << it << " ";
    std::cout << std::endl;

    std::cout << "transfer ";
    for ( uint32_t it : transferCandidate ) std::cout << it << " ";
    std::cout << std::endl;
    std::cout << std::flush;
#endif
    if ( graphicsCandidate.size() == 1 ) {
        *graphics = graphicsCandidate.front();
        presentCandidate.erase( std::remove( presentCandidate.begin(), presentCandidate.end(), *graphics ), presentCandidate.end() );
        transferCandidate.erase( std::remove( transferCandidate.begin(), transferCandidate.end(), *graphics ), transferCandidate.end() );
    }

    if ( presentCandidate.size() == 1 ) {
        *present = presentCandidate.front();
        transferCandidate.erase( std::remove( transferCandidate.begin(), transferCandidate.end(), *present ), transferCandidate.end() );
    }

    if ( transferCandidate.size() == 1 ) {
        *transfer = transferCandidate.front();
    }

    assert( *graphics != *present );
    assert( *graphics != *transfer );
    assert( *present != *transfer );
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
    const std::pmr::vector<const char*> dextension = deviceExtensions();

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

    if ( !SDL_Vulkan_CreateSurface( m_window, m_instance, &m_surface ) ) {
        assert( 0 );
        std::cout << "Failed to create sdl surface" << std::endl;
        return;
    }

    queueFamilies( m_physicalDevice
        , m_surface
        , &m_queueFamilyGraphics
        , &m_queueFamilyPresent
        , &m_queueFamilyTransfer
    );

    {
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo[ 3 ]{};
        queueCreateInfo[ 0 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[ 0 ].queueFamilyIndex = m_queueFamilyGraphics;
        queueCreateInfo[ 0 ].queueCount = 1;
        queueCreateInfo[ 0 ].pQueuePriorities = &queuePriority;

        queueCreateInfo[ 1 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[ 1 ].queueFamilyIndex = m_queueFamilyPresent;
        queueCreateInfo[ 1 ].queueCount = 1;
        queueCreateInfo[ 1 ].pQueuePriorities = &queuePriority;

        queueCreateInfo[ 2 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[ 2 ].queueFamilyIndex = m_queueFamilyTransfer;
        queueCreateInfo[ 2 ].queueCount = 1;
        queueCreateInfo[ 2 ].pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfo;
        createInfo.queueCreateInfoCount = std::size( queueCreateInfo );
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = dextension.size();
        createInfo.ppEnabledExtensionNames = dextension.data();
        createInfo.enabledLayerCount = lay.size();
        createInfo.ppEnabledLayerNames = lay.data();
        const VkResult res = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "Failed to create device" << std::endl;
            return;
        }
    }

    vkGetDeviceQueue( m_device, m_queueFamilyGraphics, 0, &m_queueGraphics );
    vkGetDeviceQueue( m_device, m_queueFamilyPresent, 0, &m_queuePresent );
    vkGetDeviceQueue( m_device, m_queueFamilyTransfer, 0, &m_queueTransfer );

    {
        VkSurfaceCapabilitiesKHR surfaceCaps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps );
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR( m_physicalDevice, m_surface, &formatCount, nullptr );
        std::pmr::vector<VkSurfaceFormatKHR> formats( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR( m_physicalDevice, m_surface, &formatCount, formats.data() );

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &presentModeCount, nullptr );
        std::pmr::vector<VkPresentModeKHR> presentModes( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &presentModeCount, presentModes.data() );

        {
            static constexpr VkSurfaceFormatKHR prefferedFormats[]{
                { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
                { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            };
            for ( const VkSurfaceFormatKHR& it : prefferedFormats ) {
                const auto found = std::find( formats.cbegin(), formats.cend(), it );
                if ( found != formats.cend() ) {
                    m_swapchainFormat = it;
                    break;
                }
            }
            assert( m_swapchainFormat != VkSurfaceFormatKHR{} );

            static constexpr VkPresentModeKHR prefferedPresents[]{
                VK_PRESENT_MODE_FIFO_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR,
            };
            for ( const VkPresentModeKHR& it : prefferedPresents ) {
                const auto found = std::find( presentModes.cbegin(), presentModes.cend(), it );
                if ( found != presentModes.cend() ) {
                    m_swapchainMode = it;
                    break;
                }
            }
        }

        VkExtent2D extent{ 1280, 920 };
        extent.width = std::clamp( extent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.height );
        extent.height = std::clamp( extent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height );
        m_swapchainExtent = extent;
        m_swapchainImageCount = std::clamp( surfaceCaps.minImageCount + 1, surfaceCaps.minImageCount, surfaceCaps.maxImageCount );

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;

        createInfo.minImageCount = m_swapchainImageCount;
        createInfo.imageFormat = m_swapchainFormat.format;
        createInfo.imageColorSpace = m_swapchainFormat.colorSpace;
        createInfo.imageExtent = m_swapchainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if ( m_queueFamilyGraphics != m_queueFamilyPresent ) {
            uint32_t queueFamilyIndices[]{ m_queueFamilyGraphics, m_queueFamilyPresent };
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = surfaceCaps.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = m_swapchainMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        const VkResult res = vkCreateSwapchainKHR( m_device, &createInfo, nullptr, &m_swapchain );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "failed to create swapchain" << std::endl;
            return;
        }

        vkGetSwapchainImagesKHR( m_device, m_swapchain, &m_swapchainImageCount, nullptr );
        m_swapchainImages.resize( m_swapchainImageCount );
        vkGetSwapchainImagesKHR( m_device, m_swapchain, &m_swapchainImageCount, m_swapchainImages.data() );

        m_swapchainImageViews.reserve( m_swapchainImageCount );
        for ( VkImage& it : m_swapchainImages ) {
            VkImageViewCreateInfo imView{};
            imView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imView.image = it;
            imView.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imView.format = m_swapchainFormat.format;
            imView.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imView.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imView.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imView.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imView.subresourceRange.baseMipLevel = 0;
            imView.subresourceRange.levelCount = 1;
            imView.subresourceRange.baseArrayLayer = 0;
            imView.subresourceRange.layerCount = 1;
            VkImageView view{};
            const VkResult res = vkCreateImageView( m_device, &imView, nullptr, &view );
            assert( res == VK_SUCCESS );
            if ( res != VK_SUCCESS ) {
                std::cout << "failed to create swapchain image view" << std::endl;
                return;
            }
            m_swapchainImageViews.emplace_back( view );
        }
    }

    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapchainFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        const VkResult res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderPass );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "failed to create render pass" << std::endl;
            return;
        }
    }

    {
        for ( const VkImageView& it : m_swapchainImageViews ) {
            VkImageView attachments[]{ it };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapchainExtent.width;
            framebufferInfo.height = m_swapchainExtent.height;
            framebufferInfo.layers = 1;
            VkFramebuffer framebuffer{};
            const VkResult res = vkCreateFramebuffer( m_device, &framebufferInfo, nullptr, &framebuffer );
            assert( res == VK_SUCCESS );
            if ( res != VK_SUCCESS ) {
                std::cout << "failed to create framebuffer" << std::endl;
                return;
            }
            m_framebuffers.emplace_back( framebuffer );
        }
    }

    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_queueFamilyGraphics;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        const VkResult res = vkCreateCommandPool( m_device, &poolInfo, nullptr, &m_commandPool );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "failed to create command pool" << std::endl;
            return;
        }
    }
    {
        m_commandBuffers.resize( m_swapchainImageCount );
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = m_swapchainImageCount;
        const VkResult res = vkAllocateCommandBuffers( m_device, &allocInfo, m_commandBuffers.data() );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "failed to allocate command buffers" << std::endl;
            return;
        }
    }
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        if ( const VkResult res = vkCreateSemaphore( m_device, &semaphoreInfo, nullptr, &m_semaphoreAvailableImage );
            res != VK_SUCCESS ) {
            assert( !"failed to create semaphore" );
            std::cout << "failed to create semaphore" << std::endl;
            return;
        }
        if ( const VkResult res = vkCreateSemaphore( m_device, &semaphoreInfo, nullptr, &m_semaphoreRender );
            res != VK_SUCCESS ) {
            assert( !"failed to create render semaphore" );
            std::cout << "failed to create render semaphore" << std::endl;
            return;
        }
    }
    m_pipelines.emplace_back( m_device
        , m_swapchainFormat.format
        , m_swapchainExtent
        , "shaders/gui_texture_color.vert.spv"
        , "shaders/gui_texture_color.frag.spv"
    );

}


RendererVK::~RendererVK()
{
    m_pipelines.clear();
    if ( m_semaphoreRender ) {
        vkDestroySemaphore( m_device, m_semaphoreRender, nullptr );
    }
    if ( m_semaphoreAvailableImage ) {
        vkDestroySemaphore( m_device, m_semaphoreAvailableImage, nullptr );
    }
    if ( m_commandPool ) {
        vkDestroyCommandPool( m_device, m_commandPool, nullptr );
    }
    for ( VkFramebuffer& it : m_framebuffers ) {
        vkDestroyFramebuffer( m_device, it, nullptr );
    }
    if ( m_renderPass ) {
        vkDestroyRenderPass( m_device, m_renderPass, nullptr );
    }
    for ( VkImageView& it : m_swapchainImageViews ) {
        vkDestroyImageView( m_device, it, nullptr );
    }
    if ( m_swapchain ) {
        vkDestroySwapchainKHR( m_device, m_swapchain, nullptr );
    }
    if ( m_surface ) {
        vkDestroySurfaceKHR( m_instance, m_surface, nullptr );
    }
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

void RendererVK::beginFrame()
{
    uint32_t imageIndex = 0;
    if ( const VkResult res = vkAcquireNextImageKHR( m_device, m_swapchain, std::numeric_limits<uint64_t>::max(), m_semaphoreAvailableImage, VK_NULL_HANDLE, &imageIndex );
        res != VK_SUCCESS ) {
        assert( !"failed to acquire image" );
        std::cout << "failed to acquire image" << std::endl;
        return;
    }
    m_currentFrame = imageIndex;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    const VkResult res = vkBeginCommandBuffer( m_commandBuffers[ m_currentFrame ], &beginInfo );
    assert( res == VK_SUCCESS );
    if ( res != VK_SUCCESS ) {
        std::cout << "failed to begin command buffer" << std::endl;
        return;
    }

    m_pipelines[ 0 ].begin(
        m_commandBuffers[ m_currentFrame ]
        , m_framebuffers[ m_currentFrame ]
        , VkRect2D{ .extent = m_swapchainExtent }
    );

}

void RendererVK::clear() {}
void RendererVK::deleteBuffer( const Buffer& ) {}
void RendererVK::deleteTexture( uint32_t ) {}
void RendererVK::push( void*, void* ) {}
void RendererVK::setViewportSize( uint32_t, uint32_t ) {}

void RendererVK::submit()
{
    m_pipelines[ 0 ].end( m_commandBuffers[ m_currentFrame ] );

    if ( const VkResult res = vkEndCommandBuffer( m_commandBuffers[ m_currentFrame ] );
        res != VK_SUCCESS ) {
        assert( !"failed to end command buffer" );
        std::cout << "failed to end command buffer" << std::endl;
        return;
    }

    VkSemaphore waitSemaphores[]{ m_semaphoreAvailableImage };
    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[ m_currentFrame ];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = renderSemaphores;
    if ( const VkResult res = vkQueueSubmit( m_queueGraphics, 1, &submitInfo, VK_NULL_HANDLE );
        res != VK_SUCCESS ) {
        assert( !"failed to submit queue" );
        std::cout << "failed to submit queue" << std::endl;
        return;
    }

}

void RendererVK::present()
{
    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkSwapchainKHR swapchain[] = { m_swapchain };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = renderSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchain;
    presentInfo.pImageIndices = &m_currentFrame;
    const VkResult res = vkQueuePresentKHR( m_queuePresent, &presentInfo );
    assert( res == VK_SUCCESS );
    if ( res != VK_SUCCESS ) {
        std::cout << "failed to present queue" << std::endl;
    }
    vkDeviceWaitIdle( m_device );
}
