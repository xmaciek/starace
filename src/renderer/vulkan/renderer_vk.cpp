#include "renderer_vk.hpp"

#include "utils_vk.hpp"

#include <SDL_vulkan.h>
#include <Tracy.hpp>

#include <algorithm>
#include <bit>
#include <cassert>
#include <iostream>

static Renderer* g_instance = nullptr;

static constexpr std::array c_enabledLayers = {
    "VK_LAYER_KHRONOS_validation",
};

static constexpr std::array c_enabledDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

constexpr std::size_t operator ""_MiB( unsigned long long v ) noexcept
{
    return v << 20;
}

static std::pmr::vector<const char*> windowExtensions( SDL_Window* window )
{
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions( window, &count, nullptr );
    std::pmr::vector<const char*> ext{};
    ext.reserve( count + 1 );
    ext.resize( count );
    SDL_Vulkan_GetInstanceExtensions( window, &count, ext.data() );
    ext.emplace_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    return ext;
}

static VkPhysicalDevice selectPhysicalDevice( VkInstance instance )
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices( instance, &count, nullptr );
    std::pmr::vector<VkPhysicalDevice> devices( count );
    vkEnumeratePhysicalDevices( instance, &count, devices.data() );

    VkPhysicalDeviceProperties deviceProperties{};
    for ( VkPhysicalDevice it : devices ) {
        vkGetPhysicalDeviceProperties( it, &deviceProperties );
        if ( deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
            return it;
        }
    }
    return VK_NULL_HANDLE;
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
    constexpr QueueCount() noexcept = default;
    constexpr QueueCount( uint32_t q, uint32_t c ) noexcept : queue{ q }, count{ c } {}
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
        if ( vec[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
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

    assert( !g_instance );
    g_instance = this;

    {
        ZoneScopedN( "create instance" );
        const VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Starace",
            .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .pEngineName = "Starace",
            .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .apiVersion = VK_API_VERSION_1_1,
        };

        const std::pmr::vector<const char*> ext = windowExtensions( m_window );

        const VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>( c_enabledLayers.size() ),
            .ppEnabledLayerNames = c_enabledLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>( ext.size() ),
            .ppEnabledExtensionNames = ext.data(),
        };

        [[maybe_unused]]
        const VkResult instanceOK = vkCreateInstance( &instanceCreateInfo, nullptr, &m_instance );
        assert( instanceOK == VK_SUCCESS );
    }

    if ( !SDL_Vulkan_CreateSurface( m_window, m_instance, &m_surface ) ) {
        assert( !"Failed to create sdl vulkan surface" );
        std::cout << "Failed to create sdl vulkan surface" << std::endl;
        std::abort();
    }

    m_debugMsg = DebugMsg{ m_instance };
    m_physicalDevice = selectPhysicalDevice( m_instance );
    assert( m_physicalDevice );

    m_depthFormat = pickSupportedFormat(
        m_physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );



    const auto [ queueGraphics, queuePresent ] = queueFamilies( m_physicalDevice, m_surface );
    m_queueFamilyGraphics = queueGraphics.queue;
    m_queueFamilyPresent = queuePresent.queue;
    {
        ZoneScopedN( "create device" );
        const std::array queuePriority{ 1.0f, 1.0f };
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

        VkPhysicalDeviceFeatures deviceFeatures{
            .wideLines = VK_TRUE,
        };
        const VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = std::size( queueCreateInfo ),
            .pQueueCreateInfos = queueCreateInfo,
            .enabledLayerCount = static_cast<uint32_t>( c_enabledLayers.size() ),
            .ppEnabledLayerNames = c_enabledLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>( c_enabledDeviceExtensions.size() ),
            .ppEnabledExtensionNames = c_enabledDeviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures,
        };
        [[maybe_unused]]
        const VkResult deviceOK = vkCreateDevice( m_physicalDevice, &createInfo, nullptr, &m_device );
        assert( deviceOK == VK_SUCCESS );
    }

    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueFamilyGraphics, m_queueFamilyPresent }
    );

    vkGetDeviceQueue( m_device, m_queueFamilyPresent, 0, &m_queuePresent );

    m_commandPool = CommandPool{ m_device, 1 + m_swapchain.imageCount() * 3, m_queueFamilyGraphics };

    {
        vkGetDeviceQueue( m_device, m_queueFamilyGraphics, 0u, &std::get<VkQueue>( m_queueGraphics ) );
        std::get<std::mutex*>( m_queueGraphics ) = &m_cmdBottleneck[ 0 ];
    }
    {
        const uint32_t idx = std::min( queueGraphics.count, 2u ) - 1u;
        assert( idx < m_cmdBottleneck.size() );
        vkGetDeviceQueue( m_device, m_queueFamilyGraphics, idx, &std::get<VkQueue>( m_queueTransfer ) );
        std::get<std::mutex*>( m_queueTransfer ) = &m_cmdBottleneck[ idx ];
    }

    m_mainPass = RenderPass{ m_device, VK_FORMAT_B8G8R8A8_UNORM, m_depthFormat };
    m_depthPrepass = RenderPass{ m_device, m_depthFormat };

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

    int i = 1;
    m_frames.resize( m_swapchain.imageCount() );
    for ( auto& it : m_frames ) {
        it.m_renderDepthTarget = RenderTarget{ RenderTarget::c_depth
            , m_physicalDevice
            , m_device
            , m_depthPrepass
            , m_swapchain.extent()
            , m_depthFormat
            , nullptr
        };
        it.m_renderTarget = RenderTarget{ RenderTarget::c_color
            , m_physicalDevice
            , m_device
            , m_mainPass
            , m_swapchain.extent()
            , VK_FORMAT_B8G8R8A8_UNORM
            , it.m_renderDepthTarget.view()
        };
        it.m_descSetUniform = DescriptorSet{ m_device, 800, 0b1, 0 };
        it.m_descSetUniformSampler = DescriptorSet{ m_device, 400, 0b1, 0b10 };
        it.m_uniformBuffer = Uniform{ m_physicalDevice, m_device, 2_MiB, 256 };
        it.m_cmdTransfer = m_commandPool[ i++ ];
        it.m_cmdDepthPrepass = m_commandPool[ i++ ];
        it.m_cmdRender = m_commandPool[ i++ ];
    }
}

void RendererVK::createPipeline( const PipelineCreateInfo& pci )
{
    ZoneScoped;

    assert( pci.m_slot < m_pipelines.size() );
    assert( pci.m_constantBindBits != 0 );
    assert( std::popcount( pci.m_constantBindBits ) == 1 );
    assert( ( pci.m_constantBindBits & pci.m_textureBindBits ) == 0 ); // mutually exclusive bits

    uint32_t descriptorSetType = pci.m_constantBindBits;
    descriptorSetType <<= 16;
    descriptorSetType |= pci.m_textureBindBits;

    DescriptorSet* descriptorSet = nullptr;
    switch ( descriptorSetType ) {
    case 0x1'0000: descriptorSet = &m_frames[ 0 ].m_descSetUniform;  break;
    case 0x1'0002: descriptorSet = &m_frames[ 0 ].m_descSetUniformSampler; break;
    default:
        assert( !"unsupported descriptor set type" );
        return;
    }

    m_pipelines[ static_cast<PipelineSlot>( pci.m_slot ) ] = PipelineVK{
        pci
        , m_device
        , m_mainPass
        , m_depthPrepass
        , descriptorSet->layout()
    };
}

RendererVK::~RendererVK()
{
    ZoneScoped;
    for ( auto& it : m_frames ) {
        it = {};
    }
    m_commandPool = {};

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

    for ( auto& it : m_pipelines ) { it = {}; }
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphoreRender );
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphoreAvailableImage );
    m_depthPrepass = {};
    m_mainPass = {};
    m_swapchain = {};
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

VkCommandBuffer RendererVK::flushUniforms()
{
    ZoneScoped;
    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    Frame& fr = m_frames[ m_currentFrame ];
    VkCommandBuffer cmd = fr.m_cmdTransfer;

    [[maybe_unused]]
    const VkResult beginOK = vkBeginCommandBuffer( cmd, &beginInfo );
    assert( beginOK == VK_SUCCESS );

    fr.m_uniformBuffer.transfer( cmd );

    [[maybe_unused]]
    const VkResult endOK = vkEndCommandBuffer( cmd );
    assert( endOK == VK_SUCCESS );
    return cmd;

}

Buffer RendererVK::createBuffer( std::pmr::vector<float>&& vec )
{
    ZoneScoped;
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, vec.size() * sizeof( float ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK* buff = new BufferVK{ m_physicalDevice, m_device, BufferVK::Purpose::eVertex, staging.sizeInBytes() };

    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    const VkBufferCopy copyRegion{
        .size = staging.sizeInBytes(),
    };

    VkCommandBuffer cmd = m_commandPool[ 0 ];
    vkBeginCommandBuffer( cmd, &beginInfo );
    vkCmdCopyBuffer( cmd, staging, *buff, 1, &copyRegion );
    vkEndCommandBuffer( cmd );

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    auto [ queue, bottleneck ] = m_queueTransfer;
    assert( queue );
    assert( bottleneck );
    {
        Bottleneck lock{ *bottleneck };
        vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        vkQueueWaitIdle( queue );
    }

    const uint32_t idx = m_bufferIndexer.next();
    [[maybe_unused]]
    BufferVK* oldBuff = m_bufferSlots[ idx ].exchange( buff );
    assert( !oldBuff );
    return idx + 1;
}

std::pmr::memory_resource* RendererVK::allocator()
{
    return std::pmr::get_default_resource();
}

static size_t formatToSize( TextureFormat fmt )
{
    switch ( fmt ) {
    case TextureFormat::eR:
        return 1;
    case TextureFormat::eRGBA:
    case TextureFormat::eBGRA:
        return 4;
    default:
        assert( !"unhandled format" );
        return 0;
    }
}

Texture RendererVK::createTexture( const TextureCreateInfo& tci, std::pmr::vector<uint8_t>&& data )
{
    ZoneScoped;
    assert( tci.width > 0 );
    assert( tci.height > 0 );
    assert( !data.empty() );

    switch ( tci.format ) {
    case TextureFormat::eR:
    case TextureFormat::eRGBA:
    case TextureFormat::eBGRA:
        break;
    default:
        assert( !"unsuported format" );
        return 0;
    }
    const std::size_t size = tci.width * tci.height * formatToSize( tci.format );
    assert( ( size + tci.dataBeginOffset ) <= data.size() );
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::Purpose::eStaging, size };
    staging.copyData( data.data() + tci.dataBeginOffset );

    TextureVK* tex = new TextureVK{ tci, m_physicalDevice, m_device };

    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    VkCommandBuffer cmd = m_commandPool[ 0 ];
    vkBeginCommandBuffer( cmd, &beginInfo );
    tex->transferFrom( cmd, staging );
    vkEndCommandBuffer( cmd );

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    {
        auto [ queue, bottleneck ] = m_queueTransfer;
        assert( queue );
        assert( bottleneck );
        Bottleneck lock{ *bottleneck };
        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( queue );
    }

    const uint32_t idx = m_textureIndexer.next();
    [[maybe_unused]]
    TextureVK* oldTex = m_textureSlots[ idx ].exchange( tex );
    assert( !oldTex );
    return idx + 1;
}

void RendererVK::beginFrame()
{
    ZoneScoped;
    m_lastLineWidth = 0.0f;
    uint32_t imageIndex = 0;
    static constexpr uint64_t timeout = 8'000'000'000; // 8 seconds
    [[maybe_unused]]
    const VkResult acquireOK = vkAcquireNextImageKHR( m_device, m_swapchain, timeout, m_semaphoreAvailableImage, VK_NULL_HANDLE, &imageIndex );
    switch ( acquireOK ) {
    case VK_SUCCESS:
    case VK_SUBOPTIMAL_KHR: // will recreate swapchain later
        break;
    default:
        assert( !"unhandled error from vkAcquireNextImageKHR" );
        break;
    }

    m_currentFrame = imageIndex;
    Frame& fr = m_frames[ imageIndex ];
    fr.m_descSetUniform.reset();
    fr.m_descSetUniformSampler.reset();
    fr.m_uniformBuffer.reset();

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

    {
        VkCommandBuffer cmd = fr.m_cmdRender;
        vkResetCommandBuffer( cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
        [[maybe_unused]]
        const VkResult cmdOK = vkBeginCommandBuffer( cmd, &beginInfo );
        assert( cmdOK == VK_SUCCESS );

        vkCmdSetViewport( cmd, 0, 1, &viewport );
        vkCmdSetScissor( cmd, 0, 1, &rect );
        m_mainPass.begin( cmd, fr.m_renderTarget.framebuffer(), rect );
    }

    {
        VkCommandBuffer cmd = fr.m_cmdDepthPrepass;
        vkResetCommandBuffer( cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
        [[maybe_unused]]
        const VkResult cmdOK = vkBeginCommandBuffer( cmd, &beginInfo );
        assert( cmdOK == VK_SUCCESS );

        vkCmdSetViewport( cmd, 0, 1, &viewport );
        vkCmdSetScissor( cmd, 0, 1, &rect );
        m_depthPrepass.begin( cmd, fr.m_renderDepthTarget.framebuffer(), rect );
    }
}

void RendererVK::deleteBuffer( Buffer b )
{
    ZoneScoped;
    assert( b != 0 );
    b--;
    BufferVK* buff = m_bufferSlots[ b ].exchange( nullptr );
    assert( buff );
    m_bufferIndexer.release( b );
    Bottleneck bottleneck{ m_bufferBottleneck };
    m_bufferPendingDelete.push_back( buff );
}

void RendererVK::deleteTexture( Texture t )
{
    ZoneScoped;
    assert( t != 0 );
    t--;
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

    for ( auto& it : m_frames ) {
        it.m_renderDepthTarget = RenderTarget{
            RenderTarget::c_depth
            , m_physicalDevice
            , m_device
            , m_depthPrepass
            , m_swapchain.extent()
            , m_depthFormat
            , nullptr
        };
        it.m_renderTarget = RenderTarget{
            RenderTarget::c_color
            , m_physicalDevice
            , m_device
            , m_mainPass
            , m_swapchain.extent()
            , VK_FORMAT_B8G8R8A8_UNORM
            , it.m_renderDepthTarget.view()
        };
    }
    vkDeviceWaitIdle( m_device );
}

void RendererVK::endFrame()
{
    ZoneScoped;
    m_lastPipeline = nullptr;

    {
        VkCommandBuffer cmd = m_frames[ m_currentFrame ].m_cmdDepthPrepass;
        m_depthPrepass.end( cmd );

        [[maybe_unused]]
        const VkResult cmdEnd = vkEndCommandBuffer( cmd );
        assert( cmdEnd == VK_SUCCESS );
    }

    VkCommandBuffer cmd = m_frames[ m_currentFrame ].m_cmdRender;
    m_mainPass.end( cmd );

    RenderTarget& mainTgt = m_frames[ m_currentFrame ].m_renderTarget;
    transferImage( cmd, mainTgt.image(), constants::fragmentOut, constants::copyFrom );
    transferImage( cmd, m_swapchain.image( m_currentFrame ), constants::undefined, constants::copyTo );

    const VkImageCopy region{
        .srcSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1 },
        .dstSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1 },
        .extent = mainTgt.extent3D()
    };

    vkCmdCopyImage( cmd
        , mainTgt.image()
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

    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    std::array cmds{
        flushUniforms()
        , m_frames[ m_currentFrame ].m_cmdDepthPrepass
        , cmd };

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = cmds.size(),
        .pCommandBuffers = cmds.data(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = renderSemaphores,
    };


    {
        auto [ queue, bottleneck ] = m_queueGraphics;
        assert( queue );
        assert( bottleneck );
        Bottleneck lock{ *bottleneck };
        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( queue );
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
    std::array waitSemaphores{ m_semaphoreRender, m_semaphoreAvailableImage };
    VkSwapchainKHR swapchain[] = { m_swapchain };
    const VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = waitSemaphores.size(),
        .pWaitSemaphores = waitSemaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = swapchain,
        .pImageIndices = &m_currentFrame,
    };

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

void RendererVK::push( const PushBuffer& pushBuffer, const void* constant )
{
    Frame& fr = m_frames[ m_currentFrame ];
    auto& descriptorPool = pushBuffer.m_texture
        ? fr.m_descSetUniformSampler
        : fr.m_descSetUniform;

    assert( pushBuffer.m_pipeline < m_pipelines.size() );
    PipelineVK& currentPipeline = m_pipelines[ pushBuffer.m_pipeline ];

    const bool rebindPipeline = m_lastPipeline != &currentPipeline;
    const bool depthWrite = currentPipeline.depthWrite();
    const bool updateLineWidth = currentPipeline.useLines() && pushBuffer.m_lineWidth != m_lastLineWidth;
    const bool bindBuffer = pushBuffer.m_vertice;
    const bool bindTexture = pushBuffer.m_texture;
    uint32_t verticeCount = pushBuffer.m_verticeCount;

    m_lastPipeline = &currentPipeline;

    const VkDescriptorBufferInfo bufferInfo = fr.m_uniformBuffer.copy( constant, currentPipeline.pushConstantSize() );
    const VkDescriptorSet descriptorSet = descriptorPool.next();
    assert( descriptorSet != VK_NULL_HANDLE );
    VkCommandBuffer cmd = fr.m_cmdRender;


    if ( rebindPipeline ) {
        if ( depthWrite ) vkCmdBindPipeline( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.depthPrepass() );
        vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline );
    }

    if ( bindTexture ) {
        const TextureVK* texture = m_textureSlots[ pushBuffer.m_texture - 1 ];
        assert( texture );
        updateDescriptor( m_device, descriptorSet, bufferInfo, texture->imageInfo() );
    }
    else {
        updateDescriptor( m_device, descriptorSet, bufferInfo );
    }

    if ( depthWrite ) vkCmdBindDescriptorSets( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
    vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );

    if ( updateLineWidth ) {
        m_lastLineWidth = pushBuffer.m_lineWidth;
        if ( depthWrite ) vkCmdSetLineWidth( fr.m_cmdDepthPrepass, pushBuffer.m_lineWidth );
        vkCmdSetLineWidth( cmd, pushBuffer.m_lineWidth );
    }

    if ( bindBuffer ) {
        const BufferVK* b = m_bufferSlots[ pushBuffer.m_vertice - 1 ];
        assert( b );
        std::array<VkBuffer, 1> buffers{ *b };
        const std::array<VkDeviceSize, 1> offsets{ 0 };
        verticeCount = b->sizeInBytes() / currentPipeline.vertexStride();
        if ( depthWrite ) vkCmdBindVertexBuffers( fr.m_cmdDepthPrepass, 0, 1, buffers.data(), offsets.data() );
        vkCmdBindVertexBuffers( cmd, 0, 1, buffers.data(), offsets.data() );
    }

    if ( depthWrite ) vkCmdDraw( fr.m_cmdDepthPrepass, verticeCount, 1, 0, 0 );
    vkCmdDraw( cmd, verticeCount, 1, 0, 0 );
}
