#include "renderer_vk.hpp"

#include "utils_vk.hpp"

#include <SDL2/SDL_vulkan.h>
#include <Tracy.hpp>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>

static Renderer* g_instance = nullptr;

constexpr std::size_t operator ""_MiB( unsigned long long v ) noexcept
{
    return v << 20;
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

template <typename T>
static std::tuple<T, T> pickDifferentValues( const std::pmr::vector<T>& a, const std::pmr::vector<T>& b )
{
    assert( !a.empty() );
    assert( !b.empty() );
    for ( const auto& it : a ) {
        const auto f = std::find( b.begin(), b.end(), it );
        if ( f != b.end() ) { continue; }
        return { it, b.front() };
    };
    for ( const auto& it : b ) {
        const auto f = std::find( a.begin(), a.end(), it );
        if ( f != a.end() ) { continue; }
        return { a.front(), it };
    };
    assert( !"unable to pick different values" );
    return {};
}

struct QueueCount {
    uint32_t queue = 0;
    uint32_t count = 0;
    constexpr bool operator == ( const QueueCount& rhs ) const
    {
        return queue == rhs.queue;
    }
};

static std::tuple<QueueCount, QueueCount> queueFamilies( VkPhysicalDevice device, VkSurfaceKHR surface )
{
    ZoneScoped;
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, nullptr );
    std::pmr::vector<VkQueueFamilyProperties> vec( count );
    std::pmr::vector<QueueCount> graphicsCandidate;
    std::pmr::vector<QueueCount> presentCandidate;
    vkGetPhysicalDeviceQueueFamilyProperties( device, &count, vec.data() );
    for ( uint32_t i = 0; i < vec.size(); ++i ) {
        if ( testBit( vec[ i ].queueFlags, VK_QUEUE_GRAPHICS_BIT, 0 ) ) {
            graphicsCandidate.emplace_back( i, vec[ i ].queueCount );
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface, &presentSupport );
        if ( presentSupport ) {
            presentCandidate.emplace_back( i, vec[ i ].queueCount );
        }
    }
    return pickDifferentValues<QueueCount>( graphicsCandidate, presentCandidate );
}

Renderer* Renderer::instance()
{
    assert( g_instance );
    return g_instance;
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
    ZoneScoped;
    // TODO: slot index offset
    { [[maybe_unused]] const auto _ = m_textureIndexer.next(); }
    { [[maybe_unused]] const auto _ = m_bufferIndexer.next(); }

    assert( !g_instance );
    g_instance = this;

    const std::pmr::vector<const char*> lay = layers();
    const std::pmr::vector<const char*> dextension = deviceExtensions();

    {
        const VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Starace",
            .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .apiVersion = VK_API_VERSION_1_1,
        };

        const std::pmr::vector<const char*> ext = extensions( m_window );
        const std::pmr::vector<const char*> lay = layers();

        const VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>( lay.size() ),
            .ppEnabledLayerNames = lay.data(),
            .enabledExtensionCount = static_cast<uint32_t>( ext.size() ),
            .ppEnabledExtensionNames = ext.data(),
        };

        const VkResult instanceOK = vkCreateInstance( &instanceCreateInfo, nullptr, &m_instance );
        assert( instanceOK == VK_SUCCESS );
    }
    m_debugMsg = DebugMsg{ m_instance };

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

    m_depthFormat = pickSupportedFormat(
        m_physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    if ( !SDL_Vulkan_CreateSurface( m_window, m_instance, &m_surface ) ) {
        assert( !"Failed to create sdl vulkan surface" );
        std::cout << "Failed to create sdl vulkan surface" << std::endl;
        return;
    }

    const auto [ queueGraphics, queuePresent ] = queueFamilies( m_physicalDevice, m_surface );
    m_queueFamilyGraphics = queueGraphics.queue;
    m_queueFamilyPresent = queuePresent.queue;

    {
        static constexpr std::array queuePriority{ 1.0f, 1.0f };
        const VkDeviceQueueCreateInfo queueCreateInfo[] {
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_queueFamilyGraphics,
                .queueCount = std::min( queueGraphics.count, 2u ),
                .pQueuePriorities = queuePriority.data(),
            },
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = m_queueFamilyPresent,
                .queueCount = 1,
                .pQueuePriorities = queuePriority.data(),
            },
        };

        static constexpr VkPhysicalDeviceFeatures deviceFeatures{
            .wideLines = VK_TRUE,
        };
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
        [[maybe_unused]]
        const VkResult deviceOK = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
        assert( deviceOK == VK_SUCCESS );
    }

    for ( auto& it : m_uniform ) {
        it = Uniform{ m_physicalDevice, m_device, 2_MiB, 256 };
    }

    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueFamilyGraphics, m_queueFamilyPresent }
    );

    vkGetDeviceQueue( m_device, m_queueFamilyPresent, 0, &m_queuePresent );

    m_graphicsCmd = CommandPool{ m_device, m_swapchain.imageCount(), m_queueFamilyGraphics, 0u };
    m_transferDataCmd = CommandPool{ m_device, m_swapchain.imageCount(), m_queueFamilyGraphics, std::min( queueGraphics.count, 2u ) - 1u };

    for ( size_t i = 0; i < m_swapchain.imageCount(); ++i ) {
        m_descriptorSetBufferSampler.emplace_back(
            m_device
            , 100
            , std::pmr::vector<DescriptorSet::UniformObject>{
                DescriptorSet::uniformBuffer,
                DescriptorSet::imageSampler
            }
        );
        m_descriptorSetBuffer.emplace_back(
            m_device
            , 800
            , std::pmr::vector<DescriptorSet::UniformObject>{
                DescriptorSet::uniformBuffer
            }
        );
    }

    m_mainPass = RenderPass{ m_device, VK_FORMAT_B8G8R8A8_UNORM, m_depthFormat };
    {
        const uint32_t imageCount = m_swapchain.imageCount();
        for ( uint32_t i = 0; i < imageCount; ++i ) {
            m_mainTargets.emplace_back(
                m_physicalDevice
                , m_device
                , m_mainPass
                , m_swapchain.extent()
                , VK_FORMAT_B8G8R8A8_UNORM
                , m_depthFormat
            );
        }
    }

    {
        static constexpr VkSemaphoreCreateInfo semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        [[maybe_unused]]
        const VkResult imgOK = vkCreateSemaphore( m_device, &semaphoreInfo, nullptr, &m_semaphoreAvailableImage );
        assert( imgOK == VK_SUCCESS );

        [[maybe_unused]]
        const VkResult renderOK = vkCreateSemaphore( m_device, &semaphoreInfo, nullptr, &m_semaphoreRender );
        assert( renderOK == VK_SUCCESS );
    }


    m_pipelines[ (size_t)Pipeline::eGuiTextureColor1 ] = PipelineVK{ Pipeline::eGuiTextureColor1
        , m_device
        , m_mainPass
        , m_descriptorSetBufferSampler[ 0 ].layout()
        , false
        , "shaders/gui_texture_color.vert.spv"
        , "shaders/gui_texture_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eLine3dStripColor ] = PipelineVK{ Pipeline::eLine3dStripColor
        , m_device
        , m_mainPass
        , m_descriptorSetBuffer[ 0 ].layout()
        , true
        , "shaders/line3_strip_color.vert.spv"
        , "shaders/line3_strip_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangleFan3dTexture ] = PipelineVK{ Pipeline::eTriangleFan3dTexture
        , m_device
        , m_mainPass
        , m_descriptorSetBufferSampler[ 0 ].layout()
        , true
        , "shaders/trianglefan_texture.vert.spv"
        , "shaders/trianglefan_texture.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangleFan3dColor ] = PipelineVK{ Pipeline::eTriangleFan3dColor
        , m_device
        , m_mainPass
        , m_descriptorSetBuffer[ 0 ].layout()
        , true
        , "shaders/trianglefan_color.vert.spv"
        , "shaders/trianglefan_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangle3dTextureNormal ] = PipelineVK{ Pipeline::eTriangle3dTextureNormal
        , m_device
        , m_mainPass
        , m_descriptorSetBufferSampler[ 0 ].layout()
        , true
        , "shaders/vert3_texture_normal3.vert.spv"
        , "shaders/vert3_texture_normal3.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eLine3dColor1 ] = PipelineVK{ Pipeline::eLine3dColor1
        , m_device
        , m_mainPass
        , m_descriptorSetBuffer[ 0 ].layout()
        , true
        , "shaders/lines_color1.vert.spv"
        , "shaders/lines_color1.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eShortString ] = PipelineVK{ Pipeline::eShortString
        , m_device
        , m_mainPass
        , m_descriptorSetBufferSampler[ 0 ].layout()
        , false
        , "shaders/short_string.vert.spv"
        , "shaders/short_string.frag.spv"
    };
}


RendererVK::~RendererVK()
{
    ZoneScoped;
    m_graphicsCmd = {};
    m_transferDataCmd = {};
    m_mainTargets.clear();
    for ( const auto& it : m_textureSlots ) {
        delete it.load();
    }
    for ( auto* it : m_texturePendingDelete ) {
        delete it;
    }

    for ( const auto& it : m_bufferSlots ) {
        delete it.load();
    }
    for ( auto* it : m_bufferPendingDelete ) {
        delete it;
    }

    for ( Uniform& it : m_uniform ) {
        it = {};
    }
    m_descriptorSetBuffer.clear();
    m_descriptorSetBufferSampler.clear();
    for ( auto& it : m_pipelines ) { it = {}; }
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphoreRender );
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphoreAvailableImage );
    m_mainPass = {};
    m_swapchain = Swapchain();

    if ( m_surface ) {
        vkDestroySurfaceKHR( m_instance, m_surface, nullptr );
    }
    if ( m_device ) {
        vkDestroyDevice( m_device, nullptr );
    }
    m_debugMsg = {};
    if ( m_instance ) {
        vkDestroyInstance( m_instance, nullptr );
    }
}

void RendererVK::flushUniforms()
{
    ZoneScoped;
    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkCommandBuffer cmd = m_transferDataCmd.buffer();
    vkBeginCommandBuffer( cmd, &beginInfo );
    m_uniform[ m_currentFrame ].transfer( cmd );
    vkEndCommandBuffer( cmd );

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    assert( m_transferDataCmd.queueIndex() < m_cmdBottleneck.size() );
    Bottleneck lock{ m_cmdBottleneck[ m_transferDataCmd.queueIndex() ] };
    VkQueue q = m_transferDataCmd.queue();
    [[maybe_unused]]
    const VkResult submitOK = vkQueueSubmit( q, 1, &submitInfo, VK_NULL_HANDLE );
    assert( submitOK == VK_SUCCESS );
    vkQueueWaitIdle( q );
}

Buffer RendererVK::createBuffer( std::pmr::vector<float>&& vec )
{
    ZoneScoped;
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( float ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK* buff = new BufferVK{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.sizeInBytes() };
    {
        assert( m_transferDataCmd.queueIndex() < m_cmdBottleneck.size() );
        Bottleneck lock{ m_cmdBottleneck[ m_transferDataCmd.queueIndex() ] };
        m_transferDataCmd.transferBufferAndWait( staging, *buff, staging.sizeInBytes() );
    }

    const uint32_t idx = m_bufferIndexer.next();
    assert( idx != 0 );
    [[maybe_unused]]
    BufferVK* oldBuff = m_bufferSlots[ idx ].exchange( buff );
    assert( !oldBuff );
    return idx;
}

std::pmr::memory_resource* RendererVK::allocator()
{
    return std::pmr::get_default_resource();
}

static VkFormat convertFormat( TextureFormat fmt )
{
    switch ( fmt ) {
    case TextureFormat::eR:
        return VK_FORMAT_R8_UNORM;
    case TextureFormat::eRGB:
    case TextureFormat::eRGBA:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::eBGR:
    case TextureFormat::eBGRA:
        return VK_FORMAT_B8G8R8A8_UNORM;
    default:
        assert( !"unhandled format" );
        return VK_FORMAT_UNDEFINED;
    }
}

static size_t formatToSize( TextureFormat fmt )
{
    switch ( fmt ) {
    case TextureFormat::eR:
        return 1;
    case TextureFormat::eRGB:
    case TextureFormat::eRGBA:
    case TextureFormat::eBGR:
    case TextureFormat::eBGRA:
        return 4;
    default:
        assert( !"unhandled format" );
        return 0;
    }
}

Texture RendererVK::createTexture( uint32_t width, uint32_t height, TextureFormat fmt, bool, std::pmr::vector<uint8_t>&& data )
{
    ZoneScoped;
    assert( width > 0 );
    assert( height > 0 );
    assert( !data.empty() );

    switch ( fmt ) {
    case TextureFormat::eRGB:
    case TextureFormat::eBGR:
    {
        std::pmr::vector<uint8_t> tmp{ width * height * 4, allocator() };
        using RGB = uint8_t[3];
        const RGB* rgb = reinterpret_cast<const RGB*>( data.data() );
        const RGB* rgbend = rgb;
        uint32_t* dst = reinterpret_cast<uint32_t*>( tmp.data() );
        std::advance( rgbend, width * height );
        std::transform( rgb, rgbend, dst, []( const RGB& rgb ) {
            return ( (uint32_t)rgb[0] << 0 )
                | ( (uint32_t)rgb[1] << 8 )
                | ( (uint32_t)rgb[2] << 16 )
                | 0xff000000;
        } );
        data = std::move( tmp );
    } break;
    default: break;
    }
    const std::size_t size = width * height * formatToSize( fmt );
    assert( size <= data.size() );
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, size };
    staging.copyData( data.data() );

    TextureVK* tex = new TextureVK{ m_physicalDevice, m_device, VkExtent2D{ width, height }, convertFormat( fmt ) };

    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkCommandBuffer cmd = m_transferDataCmd.buffer();
    vkBeginCommandBuffer( cmd, &beginInfo );
    tex->transferFrom( cmd, staging );
    vkEndCommandBuffer( cmd );

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    VkQueue queue = m_transferDataCmd.queue();
    assert( m_transferDataCmd.queueIndex() < m_cmdBottleneck.size() );
    Bottleneck lock{ m_cmdBottleneck[ m_transferDataCmd.queueIndex() ] };
    [[maybe_unused]]
    const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
    assert( submitOK == VK_SUCCESS );
    vkQueueWaitIdle( queue );

    const uint32_t idx = m_textureIndexer.next();
    assert( idx != 0 );
    [[maybe_unused]]
    TextureVK* oldTex = m_textureSlots[ idx ].exchange( tex );
    assert( !oldTex );
    return idx;
}

void RendererVK::beginFrame()
{
    ZoneScoped;
    uint32_t imageIndex = 0;
    static constexpr uint64_t timeout = 8'000'000; // 8ms
    [[maybe_unused]]
    const VkResult acquireOK = vkAcquireNextImageKHR( m_device, m_swapchain, timeout, m_semaphoreAvailableImage, VK_NULL_HANDLE, &imageIndex );
    assert( acquireOK == VK_SUCCESS );

    m_currentFrame = imageIndex;
    m_graphicsCmd.setFrame( m_currentFrame );
    m_transferDataCmd.setFrame( m_currentFrame );
    m_descriptorSetBuffer[ m_currentFrame ].reset();
    m_descriptorSetBufferSampler[ m_currentFrame ].reset();
    m_uniform[ m_currentFrame ].reset();

    const VkExtent2D extent = m_swapchain.extent();
    const VkRect2D rect{ .extent = extent };
    const VkViewport viewport{
        .x = 0,
        .y = 0,
        .width = (float)extent.width,
        .height = (float)extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    constexpr static VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    VkCommandBuffer cmd = m_graphicsCmd.buffer();
    vkResetCommandBuffer( cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
    [[maybe_unused]]
    const VkResult cmdOK = vkBeginCommandBuffer( cmd, &beginInfo );
    assert( cmdOK == VK_SUCCESS );

    vkCmdSetViewport( cmd, 0, 1, &viewport );
    vkCmdSetScissor( cmd, 0, 1, &rect );
    m_mainPass.begin( cmd, m_mainTargets[ m_currentFrame ].framebuffer(), rect );
}

void RendererVK::deleteBuffer( Buffer b )
{
    ZoneScoped;
    BufferVK* buff = m_bufferSlots[ b ].exchange( nullptr );
    assert( buff );
    m_bufferIndexer.release( b );
    Bottleneck bottleneck{ m_bufferBottleneck };
    m_bufferPendingDelete.push_back( buff );
}

void RendererVK::deleteTexture( Texture t )
{
    ZoneScoped;
    TextureVK* tex = m_textureSlots[ t ].exchange( nullptr );
    assert( tex );
    m_textureIndexer.release( t );
    Bottleneck bottleneck{ m_textureBottleneck };
    m_texturePendingDelete.push_back( tex );
}

void RendererVK::recreateSwapchain()
{
    ZoneScoped;
    vkDeviceWaitIdle( m_device );

    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueFamilyGraphics, m_queueFamilyPresent }
        , m_swapchain.steal()
    );

    const uint32_t imageCount = m_swapchain.imageCount();
    m_mainTargets.clear();
    for ( uint32_t i = 0; i < imageCount; ++i ) {
        m_mainTargets.emplace_back(
            m_physicalDevice
            , m_device
            , m_mainPass
            , m_swapchain.extent()
            , VK_FORMAT_B8G8R8A8_UNORM
            , m_depthFormat
        );
    }
    vkDeviceWaitIdle( m_device );
}

void RendererVK::endFrame()
{
    ZoneScoped;
    VkCommandBuffer cmd = m_graphicsCmd.buffer();
    m_mainPass.end( cmd );
    if ( m_lastPipeline ) {
        m_lastPipeline->end();
        m_lastPipeline = nullptr;
    }

    RenderTarget& mainTgt = m_mainTargets[ m_currentFrame ];
    transferImage( cmd, mainTgt.image().first, constants::fragmentOut, constants::copyFrom );
    transferImage( cmd, m_swapchain.image( m_currentFrame ), constants::undefined, constants::copyTo );

    const VkImageCopy region{
        .srcSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1 },
        .dstSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1 },
        .extent = mainTgt.extent3D()
    };

    vkCmdCopyImage( cmd
        , mainTgt.image().first
        , constants::copyFrom.m_layout
        , m_swapchain.image( m_currentFrame )
        , constants::copyTo.m_layout
        , 1
        , &region
    );

    transferImage( cmd, m_swapchain.image( m_currentFrame ), constants::copyTo, constants::present );

    [[maybe_unused]]
    const VkResult cmdEnd = vkEndCommandBuffer( cmd );
    assert( cmdEnd == VK_SUCCESS );

    VkSemaphore waitSemaphores[]{ m_semaphoreAvailableImage };
    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = renderSemaphores,
    };

    flushUniforms();

    {
        assert( m_graphicsCmd.queueIndex() < m_cmdBottleneck.size() );
        Bottleneck lock{ m_cmdBottleneck[ m_graphicsCmd.queueIndex() ] };
        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( m_graphicsCmd.queue(), 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( m_graphicsCmd.queue() );
    }

    decltype( m_bufferPendingDelete ) bufDel;
    {
        Bottleneck bottleneck{ m_bufferBottleneck };
        std::swap( m_bufferPendingDelete, bufDel );
    }
    for ( auto* it : bufDel ) {
        delete it;
    }

    decltype( m_texturePendingDelete ) texDel;
    {
        Bottleneck bottleneck{ m_textureBottleneck };
        std::swap( m_texturePendingDelete, texDel );
    }
    for ( auto* it : texDel ) {
        delete it;
    }

}

void RendererVK::present()
{
    ZoneScoped;
    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkSwapchainKHR swapchain[] = { m_swapchain };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = renderSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchain;
    presentInfo.pImageIndices = &m_currentFrame;

    switch ( vkQueuePresentKHR( m_queuePresent, &presentInfo ) ) {
    case VK_SUCCESS: break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        recreateSwapchain();
        break;
    default:
        assert( !"failed to present" );
    }

    vkQueueWaitIdle( m_queuePresent );
}

static void updateDescriptor( VkDevice device, VkDescriptorSet descriptorSet, const VkDescriptorBufferInfo& bufferInfo )
{
    const VkWriteDescriptorSet descriptorWrite{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };
    vkUpdateDescriptorSets( device, 1, &descriptorWrite, 0, nullptr );
}

static void updateDescriptor( VkDevice device
    , VkDescriptorSet descriptorSet
    , const VkDescriptorBufferInfo& bufferInfo
    , const VkDescriptorImageInfo& imageInfo
)
{
    const VkWriteDescriptorSet descriptorWrites[] = {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        },
    };
    const uint32_t writeCount = (uint32_t)std::size( descriptorWrites );;
    vkUpdateDescriptorSets( device, writeCount, descriptorWrites, 0, nullptr );
}

void RendererVK::push( const void* buffer, const void* constant )
{
#define CASE( TYPE ) \
    case Pipeline::TYPE: { \
        [[maybe_unused]] auto* pushBuffer = reinterpret_cast<const PushBuffer<Pipeline::TYPE>*>( buffer ); \
        [[maybe_unused]] auto* pushConstant = reinterpret_cast<const PushConstant<Pipeline::TYPE>*>( constant ); \
        static constexpr std::size_t constantSize = sizeof( PushConstant<Pipeline::TYPE> ); \
        PipelineVK& currentPipeline = m_pipelines[ (size_t)p ]; \
        if ( m_lastPipeline != &currentPipeline ) { \
            if ( m_lastPipeline ) { m_lastPipeline->end(); } \
            m_lastPipeline = &currentPipeline; \
        }

    Pipeline p = *reinterpret_cast<const Pipeline*>( buffer );
    switch ( p ) {
    CASE( eGuiTextureColor1 )
        const TextureVK* texture = m_textureSlots[ pushBuffer->m_texture ];
        if ( !texture ) {
            return;
        }

        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorImageInfo imageInfo = texture->imageInfo();
        const VkDescriptorSet descriptorSet = m_descriptorSetBufferSampler[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo, imageInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, 4, 1, 0, 0 );
    } break;

    CASE( eShortString )
        const TextureVK* texture = m_textureSlots[ pushBuffer->m_texture ];
        if ( !texture ) {
            return;
        }

        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorImageInfo imageInfo = texture->imageInfo();
        const VkDescriptorSet descriptorSet = m_descriptorSetBufferSampler[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo, imageInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eLine3dStripColor )
        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorSet descriptorSet = m_descriptorSetBuffer[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        vkCmdSetLineWidth( cmd, pushBuffer->m_lineWidth );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eLine3dColor1 )
        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorSet descriptorSet = m_descriptorSetBuffer[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        vkCmdSetLineWidth( cmd, pushBuffer->m_lineWidth );
        assert( pushBuffer->m_verticeCount <= pushConstant->m_vertices.size() );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eTriangleFan3dColor )
        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorSet descriptorSet = m_descriptorSetBuffer[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eTriangleFan3dTexture )
        const TextureVK* texture = m_textureSlots[ pushBuffer->m_texture ];
        if ( !texture ) {
            return;
        }

        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorImageInfo imageInfo = texture->imageInfo();
        const VkDescriptorSet descriptorSet = m_descriptorSetBufferSampler[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo, imageInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, pushConstant->m_vertices.size(), 1, 0, 0 );
    } break;

    CASE( eTriangle3dTextureNormal )
        const TextureVK* texture = m_textureSlots[ pushBuffer->m_texture ];
        if ( !texture ) {
            return;
        }

        const BufferVK* buffer = m_bufferSlots[ pushBuffer->m_vertices ];
        if ( !buffer ) {
            return;
        }

        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorImageInfo imageInfo = texture->imageInfo();
        const VkDescriptorSet descriptorSet = m_descriptorSetBufferSampler[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo, imageInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        currentPipeline.begin( cmd, descriptorSet );
        std::array<VkBuffer, 1> buffers{ *buffer };
        const std::array<VkDeviceSize, 1> offsets{ 0 };
        const uint32_t verticeCount = buffer->sizeInBytes() / ( 8 * sizeof( float ) );
        vkCmdBindVertexBuffers( cmd, 0, 1, buffers.data(), offsets.data() );
        vkCmdDraw( cmd, verticeCount, 1, 0, 0 );
    } break;
    default:
        break;
    }
}
