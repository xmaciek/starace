#include "renderer_vk.hpp"

#include "buffer_vk.hpp"
#include "texture_vk.hpp"

#include <SDL2/SDL_vulkan.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include "utils_vk.hpp"

static Renderer* g_instance = nullptr;

static bool operator < ( const Buffer& lhs, const Buffer& rhs ) noexcept
{
    return lhs.m_id < rhs.m_id;
}

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
    assert( !g_instance );
    g_instance = this;

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
        const VkResult res = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
        assert( res == VK_SUCCESS );
        if ( res != VK_SUCCESS ) {
            std::cout << "Failed to create device" << std::endl;
            return;
        }
    }

    for ( BufferPool& it : m_uniforms ) {
        it = BufferPool{ m_physicalDevice, m_device };
        it.reserve( sizeof( PushConstant<Pipeline::eGuiTextureColor1> ), 200 );
        it.reserve( sizeof( PushConstant<Pipeline::eShortString> ), 60 );
        it.reserve( sizeof( PushConstant<Pipeline::eTriangle3dTextureNormal> ), 3 );
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

    m_graphicsCmd = CommandPool{ m_device, m_swapchain.imageCount(), { 1, 0 }, m_queueFamilyGraphics };
    m_transferToGraphicsCmd = CommandPool{ m_device, m_swapchain.imageCount(), { 1, 1 }, m_queueFamilyGraphics };
    m_transferCmd = CommandPool{ m_device, m_swapchain.imageCount(), { 1, 0 }, m_queueFamilyTransfer };


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
    m_graphicsCmd = {};
    m_transferToGraphicsCmd = {};
    m_transferCmd = {};
    m_bufferMap.clear();
    m_bufferPendingDelete.clear();
    m_mainTargets.clear();
    for ( TextureVK* it : m_textures ) {
        delete it;
    }
    for ( TextureVK* it : m_texturesPendingDelete ) {
        delete it;
    }
    for ( BufferPool& it : m_uniforms ) {
        it = {};
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
    std::pmr::vector<BufferTransfer> pendingTransfers = std::move( m_pending );
    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VkCommandBuffer cmd = m_transferCmd.buffer();
    vkBeginCommandBuffer( cmd, &beginInfo );
    m_uniform[ m_currentFrame ].transfer( cmd );
    for ( BufferTransfer& it : pendingTransfers ) {
        const VkBufferCopy copyRegion{
            .size = it.sizeInBytes(),
        };
        vkCmdCopyBuffer( cmd, it.staging(), it.dst(), 1, &copyRegion );
    }

    vkEndCommandBuffer( cmd );
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    VkQueue q = m_transferCmd.queue();
    vkQueueSubmit( q, 1, &submitInfo, VK_NULL_HANDLE );
    vkQueueWaitIdle( q );
    m_uniforms[ m_currentFrame ].reset();
}

Buffer RendererVK::createBuffer( std::pmr::vector<float>&& vec, Buffer::Lifetime lft )
{
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( float ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK data{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.sizeInBytes() };
    m_transferCmd.transferBufferAndWait( staging, data, staging.sizeInBytes() );

    const Buffer retBuffer{ reinterpret_cast<uint64_t>( (VkBuffer)data ), lft, Buffer::Status::ePending };
    [[maybe_unused]]
    auto [ it, emplaced ] = m_bufferMap.emplace( std::make_pair( retBuffer, std::move( data ) ) );
    assert( emplaced );
    return retBuffer;
}

std::pmr::memory_resource* RendererVK::allocator()
{
    return std::pmr::get_default_resource();
}

static VkFormat convertFormat( Texture::Format fmt )
{
    switch ( fmt ) {
    case Texture::Format::eR:
        return VK_FORMAT_R8_UNORM;
    case Texture::Format::eRGB:
    case Texture::Format::eRGBA:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case Texture::Format::eBGR:
    case Texture::Format::eBGRA:
        return VK_FORMAT_B8G8R8A8_UNORM;
    default:
        assert( !"unhandled format" );
        return VK_FORMAT_UNDEFINED;
    }
}

static size_t formatToSize( Texture::Format fmt )
{
    switch ( fmt ) {
    case Texture::Format::eR:
        return 1;
    case Texture::Format::eRGB:
    case Texture::Format::eRGBA:
    case Texture::Format::eBGR:
    case Texture::Format::eBGRA:
        return 4;
    default:
        assert( !"unhandled format" );
        return 0;
    }
}

Texture RendererVK::createTexture( uint32_t width, uint32_t height, Texture::Format fmt, bool, const uint8_t* data )
{
    assert( width > 0 );
    assert( height > 0 );
    assert( data );

    std::pmr::vector<uint32_t> vec{ allocator() };
    switch ( fmt ) {
    case Texture::Format::eRGB:
    case Texture::Format::eBGR:
    {
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
    } break;
    default: break;
    }
    const std::size_t size = width * height * formatToSize( fmt );
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, size };
    staging.copyData( data );

    m_textures.emplace_back( new TextureVK{ m_physicalDevice
        , m_device
        , VkExtent2D{ width, height }
        , convertFormat( fmt )
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

    return Texture{ tex };
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
    m_graphicsCmd.setFrame( m_currentFrame );
    m_transferToGraphicsCmd.setFrame( m_currentFrame );
    m_transferCmd.setFrame( m_currentFrame );
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
    if ( const VkResult res = vkBeginCommandBuffer( cmd, &beginInfo );
        res != VK_SUCCESS ) {
        assert( !"failed to begin command buffer" );
        std::cout << "failed to begin command buffer" << std::endl;
        return;
    }
    vkCmdSetViewport( cmd, 0, 1, &viewport );
    vkCmdSetScissor( cmd, 0, 1, &rect );
    m_mainPass.begin( cmd, m_mainTargets[ m_currentFrame ].framebuffer(), rect );
}

void RendererVK::deleteBuffer( const Buffer& b )
{
    auto it = m_bufferMap.find( b );
    if ( it != m_bufferMap.end() ) {
        m_bufferPendingDelete.emplace_back( std::move( it->second ) );
        m_bufferMap.erase( it );
    }
}

void RendererVK::deleteTexture( Texture t )
{
    if ( !t.ptr ) { return; }
    TextureVK* ptr = reinterpret_cast<TextureVK*>( t.ptr );
    m_textures.erase( std::remove( m_textures.begin(), m_textures.end(), ptr ), m_textures.end() );
    m_texturesPendingDelete.push_back( ptr );
}

void RendererVK::recreateSwapchain()
{
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

void RendererVK::submit()
{
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

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = renderSemaphores;

    flushUniforms();

    [[maybe_unused]]
    const VkResult submitOK = vkQueueSubmit( m_graphicsCmd.queue(), 1, &submitInfo, VK_NULL_HANDLE );
    assert( submitOK == VK_SUCCESS );

    vkQueueWaitIdle( m_graphicsCmd.queue() );
    auto bufferDel = std::move( m_bufferPendingDelete );
    auto textureDel = std::move( m_texturesPendingDelete );
    for ( auto* it : textureDel ) {
        delete it;
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
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.ptr );
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
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.ptr );
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
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.ptr );
        assert( texture );
        if ( !texture ) { return; }

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
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.ptr );
        assert( texture );
        if ( !texture ) { return; }

        const VkDescriptorBufferInfo bufferInfo = m_uniform[ m_currentFrame ].copy( constant, constantSize );
        const VkDescriptorImageInfo imageInfo = texture->imageInfo();
        const VkDescriptorSet descriptorSet = m_descriptorSetBufferSampler[ m_currentFrame ].next();
        assert( descriptorSet != VK_NULL_HANDLE );
        updateDescriptor( m_device, descriptorSet, bufferInfo, imageInfo );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        auto vertices = m_bufferMap.find( pushBuffer->m_vertices );
        assert( vertices != m_bufferMap.end() );

        currentPipeline.begin( cmd, descriptorSet );
        std::array<VkBuffer, 1> buffers{ vertices->second };
        const std::array<VkDeviceSize, 1> offsets{ 0 };
        const uint32_t verticeCount = vertices->second.sizeInBytes() / ( 8 * sizeof( float ) );
        vkCmdBindVertexBuffers( cmd, 0, 1, buffers.data(), offsets.data() );
        vkCmdDraw( cmd, verticeCount, 1, 0, 0 );
    } break;
    default:
        break;
    }
}
