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
        it.reserve( sizeof( PushConstant<Pipeline::eLine3dStripColor> ), 300 );
        it.reserve( sizeof( PushConstant<Pipeline::eTriangle3dTextureNormal> ), 3 );
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
        , false
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/gui_texture_color.vert.spv"
        , "shaders/gui_texture_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eLine3dStripColor ] = PipelineVK{ Pipeline::eLine3dStripColor
        , m_device
        , m_mainPass
        , true
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/line3_strip_color.vert.spv"
        , "shaders/line3_strip_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangleFan3dTexture ] = PipelineVK{ Pipeline::eTriangleFan3dTexture
        , m_device
        , m_mainPass
        , true
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/trianglefan_texture.vert.spv"
        , "shaders/trianglefan_texture.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangleFan3dColor ] = PipelineVK{ Pipeline::eTriangleFan3dColor
        , m_device
        , m_mainPass
        , true
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/trianglefan_color.vert.spv"
        , "shaders/trianglefan_color.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eTriangle3dTextureNormal ] = PipelineVK{ Pipeline::eTriangle3dTextureNormal
        , m_device
        , m_mainPass
        , true
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/vert3_texture_normal3.vert.spv"
        , "shaders/vert3_texture_normal3.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eLine3dColor1 ] = PipelineVK{ Pipeline::eLine3dColor1
        , m_device
        , m_mainPass
        , true
        , m_swapchain.imageCount()
        , m_swapchain.extent()
        , "shaders/lines_color1.vert.spv"
        , "shaders/lines_color1.frag.spv"
    };
    m_pipelines[ (size_t)Pipeline::eShortString ] = PipelineVK{ Pipeline::eShortString
        , m_device
        , m_mainPass
        , false
        , m_swapchain.imageCount()
        , m_swapchain.extent()
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
    m_mainTargets.clear();
    for ( TextureVK* it : m_textures ) {
        delete it;
    }
    for ( BufferPool& it : m_uniforms ) {
        it = {};
    }

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
    const VkRect2D rect{ .extent = m_swapchain.extent() };
    m_mainPass.begin( cmd, m_mainTargets[ m_currentFrame ].framebuffer(), rect );
}

void RendererVK::clear() { }
void RendererVK::deleteBuffer( const Buffer& ) {}
void RendererVK::deleteTexture( Texture ) {}
void RendererVK::setViewportSize( uint32_t, uint32_t ) {}

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
    [[maybe_unused]]
    const VkResult presentOK = vkQueuePresentKHR( m_queuePresent, &presentInfo );
    assert( presentOK == VK_SUCCESS );
    vkQueueWaitIdle( m_queuePresent );
}

void RendererVK::push( void* buffer, void* constant )
{
#define CASE( TYPE ) \
    case Pipeline::TYPE: { \
        [[maybe_unused]] auto* pushBuffer = reinterpret_cast<PushBuffer<Pipeline::TYPE>*>( buffer ); \
        [[maybe_unused]] auto* pushConstant = reinterpret_cast<PushConstant<Pipeline::TYPE>*>( constant ); \
        PipelineVK& currentPipeline = m_pipelines[ (size_t)p ]; \
        if ( m_lastPipeline != &currentPipeline ) { \
            if ( m_lastPipeline ) { m_lastPipeline->end(); } \
            m_lastPipeline = &currentPipeline; \
        }

    Pipeline p = *reinterpret_cast<Pipeline*>( buffer );
    switch ( p ) {
    CASE( eGuiTextureColor1 )
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.m_data );
        if ( !texture ) {
            return;
        }

        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eGuiTextureColor1> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , texture->view()
            , texture->sampler()
            , descriptorSet
        );

        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, 4, 1, 0, 0 );
    } break;

    CASE( eShortString )
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.m_data );
        if ( !texture ) {
            return;
        }

        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eShortString> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , texture->view()
            , texture->sampler()
            , descriptorSet
        );

        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eLine3dStripColor )
        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eLine3dStripColor> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , VK_NULL_HANDLE
            , VK_NULL_HANDLE
            , descriptorSet
        );

        currentPipeline.begin( cmd, descriptorSet );
        vkCmdSetLineWidth( cmd, pushBuffer->m_lineWidth );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eLine3dColor1 )
        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eLine3dColor1> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , VK_NULL_HANDLE
            , VK_NULL_HANDLE
            , descriptorSet
        );

        currentPipeline.begin( cmd, descriptorSet );
        vkCmdSetLineWidth( cmd, pushBuffer->m_lineWidth );
        assert( pushBuffer->m_verticeCount <= pushConstant->m_vertices.size() );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eTriangleFan3dColor )
        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eTriangleFan3dColor> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , VK_NULL_HANDLE
            , VK_NULL_HANDLE
            , descriptorSet
        );

        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, pushBuffer->m_verticeCount, 1, 0, 0 );
    } break;

    CASE( eTriangleFan3dTexture )
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.m_data );
        assert( texture );
        if ( !texture ) { return; }


        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eTriangleFan3dTexture> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , texture->view()
            , texture->sampler()
            , descriptorSet
        );

        currentPipeline.begin( cmd, descriptorSet );
        vkCmdDraw( cmd, pushConstant->m_vertices.size(), 1, 0, 0 );
    } break;

    CASE( eTriangle3dTextureNormal )
        const TextureVK* texture = reinterpret_cast<const TextureVK*>( pushBuffer->m_texture.m_data );
        assert( texture );
        if ( !texture ) { return; }


        BufferTransfer uniform = m_uniforms[ m_currentFrame ].getBuffer( sizeof( PushConstant<Pipeline::eTriangle3dTextureNormal> ) );
        uniform.copyToStaging( reinterpret_cast<const uint8_t*>( constant ) );
        m_pending.emplace_back( uniform );

        VkCommandBuffer cmd = m_graphicsCmd.buffer();
        auto vertices = m_bufferMap.find( pushBuffer->m_vertices );
        assert( vertices != m_bufferMap.end() );

        const VkDescriptorSet descriptorSet = currentPipeline.nextDescriptor();
        assert( descriptorSet != VK_NULL_HANDLE );
        currentPipeline.updateUniforms( uniform.dst()
            , uniform.sizeInBytes()
            , texture->view()
            , texture->sampler()
            , descriptorSet
        );

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
