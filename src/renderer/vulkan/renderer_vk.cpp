#include "renderer_vk.hpp"

#include "buffer_vk.hpp"
#include "single_time_command.hpp"

#include <SDL2/SDL_vulkan.h>

#include <algorithm>
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

    std::cout << "[ FAIL ] " << data->pMessage << std::endl << std::flush;
    std::abort();
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
        appInfo.apiVersion = VK_API_VERSION_1_1;

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
        const float queuePriority = 1.0f;
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

        static constexpr VkPhysicalDeviceFeatures deviceFeatures{};
        const VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = std::size( queueCreateInfo ),
            .pQueueCreateInfos = queueCreateInfo,
            .enabledLayerCount = (uint32_t)lay.size(),
            .ppEnabledLayerNames = lay.data(),
            .enabledExtensionCount = (uint32_t)dextension.size(),
            .ppEnabledExtensionNames = dextension.data(),
            .pEnabledFeatures = &deviceFeatures,
        };
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

    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueFamilyGraphics, m_queueFamilyPresent }
    );

    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapchain.surfaceFormat().format;
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
        for ( const VkImageView& it : m_swapchain.imageViews() ) {
            assert( it != VK_NULL_HANDLE );
            VkImageView attachments[]{ it };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapchain.extent().width;
            framebufferInfo.height = m_swapchain.extent().height;
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
        m_commandBuffers.resize( m_swapchain.imageCount() );
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = m_swapchain.imageCount();
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
    m_bufferUniform0 = BufferArray( m_physicalDevice
        , m_device
        , sizeof( PushConstant<Pipeline::eGuiTextureColor1> )
        , 100
    );

    m_bufferUniformsStaging.emplace_back( m_physicalDevice
        , m_device
        , BufferVK::Purpose::eStaging
        , sizeof( PushConstant<Pipeline::eGuiTextureColor1> )
    );
    m_pipelines.emplace_back( m_device
        , m_swapchain.surfaceFormat().format
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/gui_texture_color.vert.spv"
        , "shaders/gui_texture_color.frag.spv"
    );

}


RendererVK::~RendererVK()
{
    m_bufferUniformsStaging.clear();
    m_bufferUniform0 = BufferArray();
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

    m_swapchain = Swapchain();
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

    const VkExtent2D extent = m_swapchain.extent();
    const VkViewport viewport{
        .x = 0,
        .y = (float)extent.height,
        .width = (float)extent.width,
        .height = -(float)extent.height,
    };
    constexpr static VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    VkCommandBuffer cmd = m_commandBuffers[ m_currentFrame ];
    vkResetCommandBuffer( cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
    if ( const VkResult res = vkBeginCommandBuffer( cmd, &beginInfo );
        res != VK_SUCCESS ) {
        assert( !"failed to begin command buffer" );
        std::cout << "failed to begin command buffer" << std::endl;
        return;
    }
    vkCmdSetViewport( cmd, 0, 1, &viewport );
}

void RendererVK::clear() { }
void RendererVK::deleteBuffer( const Buffer& ) {}
void RendererVK::deleteTexture( uint32_t ) {}
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
    m_bufferUniform0.reset();
    m_pipelines[ 0 ].resetDescriptors();

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

void RendererVK::push( void* buffer, void* constant )
{
    const Pipeline p = *reinterpret_cast<Pipeline*>( buffer );
    switch ( p ) {
    case Pipeline::eGuiTextureColor1: {
        VkCommandBuffer cmd = m_commandBuffers[ m_currentFrame ];
        BufferVK& staging = m_bufferUniformsStaging[ 0 ];
        staging.copyData( reinterpret_cast<const uint8_t*>( constant ) );
        VkBuffer uniform = m_bufferUniform0.next();
        {
            const VkBufferCopy copyRegion{
                .size = sizeof( PushConstant<Pipeline::eGuiTextureColor1> ),
            };
            SingleTimeCommand cmd{ m_device, m_commandPool, m_queueGraphics };
            vkCmdCopyBuffer( cmd, staging, uniform, 1, &copyRegion );
        }

        const VkDescriptorSet descriptorSet = m_pipelines[ 0 ].nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        m_pipelines[ 0 ].updateUniforms( uniform
            , m_bufferUniform0.size()
            , VK_NULL_HANDLE
            , VK_NULL_HANDLE
            , descriptorSet
        );
        VkRect2D renderArea{};
        renderArea.extent = m_swapchain.extent();
        m_pipelines[ 0 ].begin( cmd
            , m_framebuffers[ m_currentFrame ]
            , renderArea
            , descriptorSet
        );
        vkCmdDraw( cmd, 4, 1, 0, 0 );
    } break;

    default:
        break;
    }
}
