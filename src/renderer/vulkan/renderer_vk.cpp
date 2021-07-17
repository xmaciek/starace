#include "renderer_vk.hpp"

#include "buffer_vk.hpp"
#include "single_time_command.hpp"
#include "texture_vk.hpp"

#include <SDL2/SDL_vulkan.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>

Renderer* Renderer::s_instance = nullptr;
static PFN_vkCreateDebugUtilsMessengerEXT createDebugUtilsMessengerEXT{};
static PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT{};

static bool operator < ( const Buffer& lhs, const Buffer& rhs ) noexcept
{
    return lhs.m_id < rhs.m_id;
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
        static constexpr std::array<float, 4> queuePriority{ 1.0f, 1.0f, 1.0f, 1.0f };
        VkDeviceQueueCreateInfo queueCreateInfo[ 3 ]{};
        queueCreateInfo[ 0 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[ 0 ].queueFamilyIndex = m_queueFamilyGraphics;
        queueCreateInfo[ 0 ].queueCount = 2;
        queueCreateInfo[ 0 ].pQueuePriorities = queuePriority.data();

        queueCreateInfo[ 1 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[ 1 ].queueFamilyIndex = m_queueFamilyPresent;
        queueCreateInfo[ 1 ].queueCount = 1;
        queueCreateInfo[ 1 ].pQueuePriorities = queuePriority.data();

        queueCreateInfo[ 2 ].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[ 2 ].queueFamilyIndex = m_queueFamilyTransfer;
        queueCreateInfo[ 2 ].queueCount = 1;
        queueCreateInfo[ 2 ].pQueuePriorities = queuePriority.data();

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

    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueFamilyGraphics, m_queueFamilyPresent }
    );

    vkGetDeviceQueue( m_device, m_queueFamilyPresent, 0, &m_queuePresent );

    m_graphicsCmd = CommandPool{ m_device, m_swapchain.imageCount(), { 1, 0 }, m_queueFamilyGraphics };
    m_transferToGraphicsCmd = CommandPool{ m_device, m_swapchain.imageCount(), { 1, 1 }, m_queueFamilyGraphics };
    m_transferCmd = CommandPool{ m_device, m_swapchain.imageCount(), { 1, 0 }, m_queueFamilyTransfer };

    {
        const VkAttachmentDescription colorAttachment{
            .format = m_swapchain.surfaceFormat().format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };
        const VkAttachmentDescription depthAttachment{
            .format = m_swapchain.depthFormat(),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        static constexpr VkAttachmentReference colorAttachmentRef{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        static constexpr VkAttachmentReference depthAttachmentRef{
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        static constexpr VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentRef,
            .pDepthStencilAttachment = &depthAttachmentRef,
        };

        static constexpr VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        };

        const std::array attachments = { colorAttachment, depthAttachment };
        const VkRenderPassCreateInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency,
        };

        const VkResult res = vkCreateRenderPass( m_device, &renderPassInfo, nullptr, &m_renderPass );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "failed to create render pass" << std::endl;
            return;
        }
    }

    {
        const std::pmr::vector<VkImageView>& imageViews = m_swapchain.imageViews();
        const std::pmr::vector<VkImageView>& depthViews = m_swapchain.depthViews();
        const uint32_t imageCount = m_swapchain.imageCount();
        assert( imageCount == imageViews.size() );
        assert( imageCount == depthViews.size() );
        for ( uint32_t i = 0; i < imageCount; ++i ) {
            std::array attachments = { imageViews[ i ], depthViews[ i ] };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_swapchain.extent().width;
            framebufferInfo.height = m_swapchain.extent().height;
            framebufferInfo.layers = 1;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            [[maybe_unused]]
            const VkResult frameBufferOK = vkCreateFramebuffer( m_device, &framebufferInfo, nullptr, &framebuffer );
            assert( frameBufferOK == VK_SUCCESS );
            m_framebuffers.emplace_back( framebuffer );
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
        , 200
    );
    m_bufferUniform1 = BufferArray( m_physicalDevice
        , m_device
        , sizeof( PushConstant<Pipeline::eTriangle3dTextureNormal> )
        , 10
    );

    m_bufferUniformsStaging.emplace_back( m_physicalDevice
        , m_device
        , BufferVK::Purpose::eStaging
        , sizeof( PushConstant<Pipeline::eGuiTextureColor1> )
    );
    m_bufferUniformsStaging.emplace_back( m_physicalDevice
        , m_device
        , BufferVK::Purpose::eStaging
        , sizeof( PushConstant<Pipeline::eTriangle3dTextureNormal> )
    );
    m_clear = Clear{ m_device, m_swapchain.surfaceFormat().format, m_swapchain.depthFormat(), false };
    m_presentTransfer = Clear{ m_device, m_swapchain.surfaceFormat().format, m_swapchain.depthFormat(), true };
    m_pipelines[ (size_t)Pipeline::eGuiTextureColor1 ] = PipelineVK{ Pipeline::eGuiTextureColor1
        , m_device
        , m_swapchain.surfaceFormat().format
        , m_swapchain.depthFormat()
        , false
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/gui_texture_color.vert.spv"
        , "shaders/gui_texture_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangle3dTextureNormal ] = PipelineVK{ Pipeline::eTriangle3dTextureNormal
        , m_device
        , m_swapchain.surfaceFormat().format
        , m_swapchain.depthFormat()
        , true
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/vert3_texture_normal3.vert.spv"
        , "shaders/vert3_texture_normal3.frag.spv"
    };

}


RendererVK::~RendererVK()
{
    m_graphicsCmd = {};
    m_transferToGraphicsCmd = {};
    m_transferCmd = {};
    m_clear = {};
    m_presentTransfer = {};
    m_bufferMap.clear();
    m_bufferMap2.clear();
    m_bufferMap3.clear();
    m_bufferMap4.clear();
    for ( TextureVK* it : m_textures ) {
        delete it;
    }
    m_bufferUniformsStaging.clear();
    m_bufferUniform0 = BufferArray();
    m_bufferUniform1 = BufferArray();
    for ( auto& it : m_pipelines ) { it = {}; }
    if ( m_semaphoreRender ) {
        vkDestroySemaphore( m_device, m_semaphoreRender, nullptr );
    }
    if ( m_semaphoreAvailableImage ) {
        vkDestroySemaphore( m_device, m_semaphoreAvailableImage, nullptr );
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

Buffer RendererVK::createBuffer( std::pmr::vector<float>&& vec, Buffer::Lifetime lft )
{
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( float ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK data{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.size() };
    m_transferCmd.transferBufferAndWait( staging, data, staging.size() );

    const Buffer retBuffer{ reinterpret_cast<uint64_t>( (VkBuffer)data ), lft, Buffer::Status::ePending };
    auto [ it, emplaced ] = m_bufferMap.emplace( std::make_pair( retBuffer, std::move( data ) ) );
    assert( emplaced );
    return retBuffer;
}

Buffer RendererVK::createBuffer( std::pmr::vector<glm::vec2>&& vec, Buffer::Lifetime lft )
{
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( glm::vec2 ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK data{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.size() };
    m_transferCmd.transferBufferAndWait( staging, data, staging.size() );

    const Buffer retBuffer{ reinterpret_cast<uint64_t>( (VkBuffer)data ), lft, Buffer::Status::ePending };
    m_bufferMap2.emplace( std::make_pair( retBuffer, std::move( data ) ) );
    return retBuffer;
}

Buffer RendererVK::createBuffer( std::pmr::vector<glm::vec3>&& vec, Buffer::Lifetime lft )
{
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( glm::vec3 ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK data{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.size() };
    m_transferCmd.transferBufferAndWait( staging, data, staging.size() );

    const Buffer retBuffer{ reinterpret_cast<uint64_t>( (VkBuffer)data ), lft, Buffer::Status::ePending };
    m_bufferMap3.emplace( std::make_pair( retBuffer, std::move( data ) ) );
    return retBuffer;
}

Buffer RendererVK::createBuffer( std::pmr::vector<glm::vec4>&& vec, Buffer::Lifetime lft )
{
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( glm::vec4 ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK data{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.size() };
    m_transferCmd.transferBufferAndWait( staging, data, staging.size() );

    const Buffer retBuffer{ reinterpret_cast<uint64_t>( (VkBuffer)data ), lft, Buffer::Status::ePending };
    m_bufferMap4.emplace( std::make_pair( retBuffer, std::move( data ) ) );
    return retBuffer;
}

std::pmr::memory_resource* RendererVK::allocator()
{
    return std::pmr::get_default_resource();
}

Texture RendererVK::createTexture( uint32_t width, uint32_t height, Texture::Format fmt, bool, const uint8_t* data )
{
    assert( width > 0 );
    assert( height > 0 );
    assert( data );

    std::pmr::vector<uint32_t> vec{ allocator() };
    if ( fmt == Texture::Format::eRGB ) {
        fmt = Texture::Format::eRGBA;
        vec.resize( width * height );
        using RGB = uint8_t[3];
        const RGB* rgb = reinterpret_cast<const RGB*>( data );
        const RGB* rgbend = rgb;
        std::advance( rgbend, width * height );
        std::transform( rgb, rgbend, vec.begin(), []( const RGB& rgb ) {
            return ( (uint32_t)rgb[0] << 0 )
                | ( (uint32_t)rgb[1] << 8 )
                | ( (uint32_t)rgb[2] << 16 )
                | 0xff000000;
        } );
        data = reinterpret_cast<const uint8_t*>( vec.data() );
    }
    const std::size_t size = width * height * ( fmt == Texture::Format::eRGB ? 3 : 4 );
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, size };
    staging.copyData( data );

    m_textures.emplace_back( new TextureVK{ m_physicalDevice
        , m_device
        , VkExtent2D{ width, height }
        , fmt == Texture::Format::eRGB
            ? VK_FORMAT_R8G8B8_SRGB
            : VK_FORMAT_R8G8B8A8_UNORM
    } );
    TextureVK* tex = m_textures.back();

    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkCommandBuffer cmd = m_transferToGraphicsCmd.buffer();
    vkBeginCommandBuffer( cmd, &beginInfo );
    tex->transferFrom( cmd, staging );
    vkEndCommandBuffer( cmd );

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    VkQueue queue = m_transferToGraphicsCmd.queue();
    vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( queue );

    return Texture{ reinterpret_cast<uint64_t>( tex ) };
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
    m_graphicsCmd.setFrame( imageIndex );
    m_transferToGraphicsCmd.setFrame( imageIndex );
    m_transferCmd.setFrame( imageIndex );

    const VkExtent2D extent = m_swapchain.extent();
    const VkViewport viewport{
        .x = 0,
        .y = (float)extent.height,
        .width = (float)extent.width,
        .height = -(float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    constexpr static VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    VkCommandBuffer cmd = m_graphicsCmd.buffer();
    vkResetCommandBuffer( cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
    if ( const VkResult res = vkBeginCommandBuffer( cmd, &beginInfo );
        res != VK_SUCCESS ) {
        assert( !"failed to begin command buffer" );
        std::cout << "failed to begin command buffer" << std::endl;
        return;
    }
    vkCmdSetViewport( cmd, 0, 1, &viewport );
    m_clear( cmd, m_framebuffers[ m_currentFrame ], VkRect2D{ .extent = m_swapchain.extent() } );
}

void RendererVK::clear() { }
void RendererVK::deleteBuffer( const Buffer& ) {}
void RendererVK::deleteTexture( Texture ) {}
void RendererVK::setViewportSize( uint32_t, uint32_t ) {}

void RendererVK::submit()
{
    VkCommandBuffer cmd = m_graphicsCmd.buffer();
    if ( m_lastPipeline ) {
        m_lastPipeline->end( cmd );
        m_lastPipeline = nullptr;
    }
    m_presentTransfer( cmd, m_framebuffers[ m_currentFrame ], VkRect2D{ .extent = m_swapchain.extent() } );
    if ( const VkResult res = vkEndCommandBuffer( cmd );
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
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = renderSemaphores;
    if ( const VkResult res = vkQueueSubmit( m_graphicsCmd.queue(), 1, &submitInfo, VK_NULL_HANDLE );
        res != VK_SUCCESS ) {
        assert( !"failed to submit queue" );
        std::cout << "failed to submit queue" << std::endl;
        return;
    }
    m_bufferUniform0.reset();
    m_bufferUniform1.reset();
    for ( auto& pipeline : m_pipelines ) {
        pipeline.resetDescriptors();
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

void RendererVK::push( void* buffer, void* constant )
{
#define CASE( TYPE ) \
    case Pipeline::TYPE: { \
        [[maybe_unused]] auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::TYPE>*>( buffer ); \
        [[maybe_unused]] auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::TYPE>*>( constant ); \
        PipelineVK& currentPipeline = m_pipelines[ (size_t)p ]; \
        if ( m_lastPipeline != &currentPipeline ) { \
            if ( m_lastPipeline ) { m_lastPipeline->end( m_graphicsCmd.buffer() ); } \
            m_lastPipeline = &currentPipeline; \
        }

    Pipeline p = *reinterpret_cast<Pipeline*>( buffer );
    switch ( p ) {
    CASE( eGuiTextureColor1 )
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.m_data );
        if ( !texture ) {
            return;
        }

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        BufferVK& staging = m_bufferUniformsStaging[ 0 ];
        staging.copyData( reinterpret_cast<const uint8_t*>( constant ) );
        VkBuffer uniform = m_bufferUniform0.next();

        m_transferCmd.transferBufferAndWait( staging, uniform, sizeof( PushConstant<Pipeline::eGuiTextureColor1> ) );

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform
            , m_bufferUniform0.size()
            , texture->view()
            , texture->sampler()
            , descriptorSet
        );
        VkRect2D renderArea{};
        renderArea.extent = m_swapchain.extent();
        currentPipeline.begin( cmd
            , m_framebuffers[ m_currentFrame ]
            , renderArea
            , descriptorSet
        );
        vkCmdDraw( cmd, 4, 1, 0, 0 );
    } break;

    CASE( eTriangle3dTextureNormal )
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.m_data );
        assert( texture );
        if ( !texture ) { return; }

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        auto vertices = m_bufferMap.find( pushBuffer->m_vertices );
        assert( vertices != m_bufferMap.end() );

        BufferVK& staging = m_bufferUniformsStaging[ 1 ];
        staging.copyData( reinterpret_cast<const uint8_t*>( constant ) );
        VkBuffer uniform = m_bufferUniform1.next();
        m_transferCmd.transferBufferAndWait( staging, uniform, staging.size() );

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform
            , m_bufferUniform1.size()
            , texture->view()
            , texture->sampler()
            , descriptorSet
        );
        const VkRect2D renderArea{
            .extent = m_swapchain.extent()
        };
        currentPipeline.begin( cmd
            , m_framebuffers[ m_currentFrame ]
            , renderArea
            , descriptorSet
        );
        std::array<VkBuffer, 1> buffers{ vertices->second };
        const std::array<VkDeviceSize, 1> offsets{ 0 };
        const uint32_t verticeCount = vertices->second.size() / ( 8 * sizeof( float ) );
        vkCmdBindVertexBuffers( cmd, 0, 1, buffers.data(), offsets.data() );
        vkCmdDraw( cmd, verticeCount, 1, 0, 0 );
    } break;
    default:
        break;
    }
}
