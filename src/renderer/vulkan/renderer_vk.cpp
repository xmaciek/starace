#include "renderer_vk.hpp"

#include "utils_vk.hpp"

#include <platform/utils.hpp>

#include <SDL_vulkan.h>
#include <profiler.hpp>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>
#include <iostream>

static class RendererSetup {
public:
    RendererSetup()
    {
        Renderer::windowFlag = SDL_WINDOW_VULKAN;
        Renderer::create = []( const Renderer::CreateInfo& ci ) -> Renderer*
        {
            return new RendererVK{ ci };
        };
    }
} setup{};

struct ResourceDeleter {
    void operator () ( TextureVK* t ) { assert( t ); delete t; }
    void operator () ( BufferVK* b ) { assert( b ); delete b; }
};

// arbitrary values
static constexpr uint32_t INVALID_INDEX = 0xFFFF'FFFFu;
static constexpr uint32_t BUFFER_ID_CHECK = 0x1F00'0000u;
static constexpr uint32_t TEXTURE_ID_CHECK = 0x2F00'0000u;
static constexpr uint32_t INDEX_BITS = 0xFFFFu;
static constexpr uint32_t DATA_BITS = 0xFF'0000u;
template <uint32_t TMask>
requires ( TMask > 0 && ( TMask & INDEX_BITS ) == 0 )
static constexpr uint32_t index2Id( uint32_t index ) { return index | TMask; }

template <uint32_t TMask>
requires ( TMask > 0 && ( TMask & INDEX_BITS ) == 0 )
static uint32_t id2Index( uint32_t id )
{
    const uint32_t MASK = INDEX_BITS | DATA_BITS;
    if ( ( id & ~MASK ) == TMask ) [[likely]]
        return id & INDEX_BITS;
    return INVALID_INDEX;
}

static constexpr auto& bufferIndexToId = index2Id<BUFFER_ID_CHECK>;
static constexpr auto& bufferIdToIndex = id2Index<BUFFER_ID_CHECK>;
static constexpr auto& textureIdToIndex = id2Index<TEXTURE_ID_CHECK>;
static constexpr Texture textureIndexToId( uint32_t index, uint32_t channelCount )
{
    assert( channelCount <= 4 );
    return TEXTURE_ID_CHECK | channelCount << 16 | index;
}

static Renderer* g_instance = nullptr;

constexpr std::size_t operator ""_MiB( unsigned long long v ) noexcept
{
    return v << 20;
}

static std::pmr::vector<const char*> windowExtensions( SDL_Window* window )
{
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions( window, &count, nullptr );
    std::pmr::vector<const char*> ret( count );
    SDL_Vulkan_GetInstanceExtensions( window, &count, ret.data() );
    return ret;
}

static VkPhysicalDevice selectPhysicalDevice( VkInstance instance, VkPhysicalDeviceProperties* deviceProperties )
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices( instance, &count, nullptr );
    std::pmr::vector<VkPhysicalDevice> devices( count );
    vkEnumeratePhysicalDevices( instance, &count, devices.data() );

    for ( VkPhysicalDevice it : devices ) {
        vkGetPhysicalDeviceProperties( it, deviceProperties );
        if ( deviceProperties->deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
            return it;
        }
    }
    if ( devices.empty() ) platform::showFatalError( "Vulkan error", "No GPU found" );
    return devices.front();
}

struct FormatSupportTest {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    struct Info {
        VkFormat format{};
        VkImageTiling tiling{};
        VkFormatFeatureFlags flags{};
    };

    bool operator ()( const Info& fi ) const
    {
        VkFormatProperties props{};
        vkGetPhysicalDeviceFormatProperties( physicalDevice, fi.format, &props );
        switch ( fi.tiling ) {
        case VK_IMAGE_TILING_LINEAR: return ( props.linearTilingFeatures & fi.flags ) == fi.flags;
        case VK_IMAGE_TILING_OPTIMAL: return ( props.optimalTilingFeatures & fi.flags ) == fi.flags;
        default: assert( !"wrong tiling requested" ); return false;
        }
    }

    static VkFormat pick( std::span<const Info> table, VkPhysicalDevice physicalDevice )
    {
        auto it = std::find_if( table.begin(), table.end(), FormatSupportTest{ physicalDevice } );
        assert( it != table.end() );
        assert( it->format != VK_FORMAT_UNDEFINED );
        return it->format;
    };
};


Renderer* Renderer::instance()
{
    assert( g_instance );
    return g_instance;
}

RendererVK::RendererVK( const Renderer::CreateInfo& createInfo )
: m_window{ createInfo.window }
{
    ZoneScoped;
    assert( !g_instance );
    g_instance = this;

    m_instance = Instance{ windowExtensions( createInfo.window ) };

    if ( !SDL_Vulkan_CreateSurface( m_window, m_instance, &m_surface ) ) {
        platform::showFatalError( "Vulkan error", "Failed to create SDL Vulkan surface" );
        return;
    }

    m_debugMsg = DebugMsg{ m_instance };

    VkPhysicalDeviceProperties physicalProperties{};
    m_physicalDevice = selectPhysicalDevice( m_instance, &physicalProperties );
    assert( m_physicalDevice );

    {
        using Info = FormatSupportTest::Info;

        static constexpr std::array colorFormatWishlist{
            Info{ VK_FORMAT_B10G11R11_UFLOAT_PACK32,  VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
            Info{ VK_FORMAT_R16G16B16A16_UNORM,       VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
            Info{ VK_FORMAT_B8G8R8A8_UNORM,           VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
            Info{ VK_FORMAT_R8G8B8A8_UNORM,           VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
        };
        static constexpr std::array depthFormatWishlist{
            Info{ VK_FORMAT_D24_UNORM_S8_UINT,    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
            Info{ VK_FORMAT_D32_SFLOAT,           VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
            Info{ VK_FORMAT_D32_SFLOAT_S8_UINT,   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT },
        };

        m_colorFormat = FormatSupportTest::pick( colorFormatWishlist, m_physicalDevice );
        m_depthFormat = FormatSupportTest::pick( depthFormatWishlist, m_physicalDevice );
    }

    m_queueManager = QueueManager{ m_physicalDevice, m_surface };

    {
        auto layers = m_instance.layers();
        auto queues = m_queueManager.createInfo();
        m_device = Device{ m_physicalDevice, layers, queues };
    }

    m_queueManager.acquire( m_device );
    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueManager.graphicsFamily(), m_queueManager.presentFamily() }
        , createInfo.vsync
    );

    m_mainPass = RenderPass{ m_device, RenderPass::eColor };
    m_depthPrepass = RenderPass{ m_device, RenderPass::eDepth };

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

    m_transferCommandPool = CommandPool{ m_device, 1, m_queueManager.transferFamily() };
    m_transferCmd = m_transferCommandPool[ 0 ];

    m_frames.resize( m_swapchain.imageCount() );

    for ( auto& it : m_frames ) {
        it.m_uniformBuffer = Uniform{ m_physicalDevice, m_device, 2_MiB, physicalProperties.limits.minUniformBufferOffsetAlignment };
        it.m_commandPool = CommandPool{ m_device, 3, m_queueManager.graphicsFamily() };
        it.m_cmdUniform = it.m_commandPool[ 0 ];
        it.m_cmdDepthPrepass = it.m_commandPool[ 1 ];
        it.m_cmdColorPass = it.m_commandPool[ 2 ];
    }
    recreateRenderTargets( m_swapchain.extent() );

    {
        static constexpr uint64_t pink = 0x00000000'F81FF81Full;
        static constexpr uint64_t black = 0ull;
        static constexpr uint64_t texels[ 64 ]{
            pink, black, pink, black, pink, black, pink, black,
            black, pink, black, pink, black, pink, black, pink,
            pink, black, pink, black, pink, black, pink, black,
            black, pink, black, pink, black, pink, black, pink,
            pink, black, pink, black, pink, black, pink, black,
            black, pink, black, pink, black, pink, black, pink,
            pink, black, pink, black, pink, black, pink, black,
            black, pink, black, pink, black, pink, black, pink,
        };
        TextureCreateInfo tci{
            .width = 32,
            .height = 32,
            .mip0ByteCount = sizeof( texels ),
            .format = TextureFormat::eBC1_unorm,
            .u = TextureAddressMode::eRepeat,
            .v = TextureAddressMode::eRepeat,
        };
        m_defaultTextureId = createTexture( tci, std::span<const uint8_t>{ reinterpret_cast<const uint8_t*>( &texels ), sizeof( texels ) } );
        m_defaultTexture = m_textureSlots[ textureIdToIndex( m_defaultTextureId ) ];
        assert( m_defaultTexture );
    }
}

PipelineSlot RendererVK::createPipeline( const PipelineCreateInfo& pci )
{
    ZoneScoped;

    const PipelineSlot slot = static_cast<PipelineSlot>( m_pipelineIndexer.next() );

    auto findOrAddDescriptorId = []( auto& array, uint64_t bindpoints ) -> std::tuple<uint32_t, bool>
    {
        auto it = std::find( array.begin(), array.end(), bindpoints );
        if ( it != array.end() ) {
            return { static_cast<uint32_t>( std::distance( array.begin(), it ) ), false };
        }
        it = std::find( array.begin(), array.end(), 0 );
        assert( it != array.end() );
        *it = bindpoints;
        return { static_cast<uint32_t>( std::distance( array.begin(), it ) ), true };
    };

    uint64_t binding = DescriptorSet::createBindingID( pci );
    auto [ descriptorId, add ] = findOrAddDescriptorId( m_pipelineDescriptorIds, binding );
    if ( add ) {
        for ( auto& fr : m_frames ) {
            fr.m_descriptorSets[ descriptorId ] = DescriptorSet{ m_device, pci };
        }
    }

    VkDescriptorSetLayout layout = m_frames[ 0 ].m_descriptorSets[ descriptorId ].layout();
    m_pipelines[ slot ] = PipelineVK{
        pci
        , m_device
        , m_depthFormat
        , m_colorFormat
        , layout
        , descriptorId
    };
    return slot;
}

RendererVK::~RendererVK()
{
    ZoneScoped;
    std::ranges::for_each( m_frames, []( auto& f ) { f = {}; } );
    m_transferCommandPool = {};

    std::ranges::for_each( m_textureSlots, []( auto& t ) { delete t.load(); } );
    std::ranges::for_each( m_bufferSlots, []( auto& b ) { delete b.load(); } );
    std::ranges::for_each( m_resourceDelete, []( auto& a ) { std::visit( ResourceDeleter{}, a ); } );
    std::ranges::for_each( m_pipelines, []( auto& p ) { p = {}; } );
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphoreRender );
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphoreAvailableImage );
    m_depthPrepass = {};
    m_mainPass = {};
    m_swapchain = {};
    if ( m_surface ) {
        vkDestroySurfaceKHR( m_instance, m_surface, nullptr );
    }
}

void RendererVK::setVSync( VSync v )
{
    m_pendingVSyncChange = v;
}

bool RendererVK::featureAvailable( Feature f ) const
{
    assert( m_physicalDevice );
    assert( m_surface );
    switch ( f ) {
    case Feature::eVSyncMailbox:
        return Swapchain::supportedVSyncs( m_physicalDevice, m_surface )[ (uint32_t)VSync::eMailbox ];
    case Feature::eVRSAA:
        return m_device.hasFeature( Device::eVRS );
    default:
        return false;
    }
}

void RendererVK::setFeatureEnabled( Feature f, bool b )
{
    switch ( f ) {
    case Feature::eVSyncMailbox: break;
    case Feature::eVRSAA:
        m_mainPass.enableVRS( featureAvailable( f ) && b );
        refreshResolution();
        break;
    default:
        assert( !"unhandled enum" );
        break;
    }
}

static void beginRecording( VkCommandBuffer cmd )
{
    static constexpr VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    [[maybe_unused]]
    const VkResult resetOK = vkResetCommandBuffer( cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );
    assert( resetOK == VK_SUCCESS );

    [[maybe_unused]]
    const VkResult cmdOK = vkBeginCommandBuffer( cmd, &beginInfo );
    assert( cmdOK == VK_SUCCESS );
}

Buffer RendererVK::createBuffer( std::span<const float> vec )
{
    ZoneScoped;
    BufferVK staging{ m_physicalDevice, m_device, BufferVK::STAGING, static_cast<uint32_t>( vec.size() * sizeof( float ) ) };
    staging.copyData( reinterpret_cast<const uint8_t*>( vec.data() ) );

    BufferVK* buff = new BufferVK{ m_physicalDevice, m_device, BufferVK::DEVICE_LOCAL, staging.sizeInBytes() };

    const VkBufferCopy copyRegion{
        .size = staging.sizeInBytes(),
    };

    VkCommandBuffer cmd = m_transferCmd;
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    {
        ZoneScopedN( "queue submit" );
        auto [ queue, bottleneck ] = m_queueManager.transfer();
        assert( queue );
        assert( bottleneck );
        Bottleneck lock{ *bottleneck };

        m_transferCommandPool.reset();
        beginRecording( cmd );
        vkCmdCopyBuffer( cmd, staging, *buff, 1, &copyRegion );
        vkEndCommandBuffer( cmd );

        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( queue );
    }

    const uint32_t idx = static_cast<uint32_t>( m_bufferIndexer.next() );
    [[maybe_unused]]
    BufferVK* oldBuff = m_bufferSlots[ idx ].exchange( buff );
    assert( !oldBuff );
    return bufferIndexToId( idx );
}

Texture RendererVK::createTexture( const TextureCreateInfo& tci, std::span<const uint8_t> data )
{
    ZoneScoped;
    assert( tci.width > 0 );
    assert( tci.height > 0 );
    assert( !data.empty() );

    const uint32_t size = static_cast<uint32_t>( data.size() );

    BufferVK staging{ m_physicalDevice, m_device, BufferVK::STAGING, size };
    staging.copyData( data.data() );

    TextureVK* tex = new TextureVK{ tci, m_physicalDevice, m_device };

    VkCommandBuffer cmd = m_transferCmd;
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    {
        ZoneScopedN( "queue submit" );
        auto [ queue, bottleneck ] = m_queueManager.transfer();
        assert( queue );
        assert( bottleneck );
        Bottleneck lock{ *bottleneck };

        m_transferCommandPool.reset();
        beginRecording( cmd );
        tex->transferFrom( cmd, staging, tci.mip0ByteCount );
        vkEndCommandBuffer( cmd );

        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( queue );
    }

    const uint32_t idx = static_cast<uint32_t>( m_textureIndexer.next() );
    [[maybe_unused]]
    TextureVK* oldTex = m_textureSlots[ idx ].exchange( tex );
    assert( !oldTex );

    return textureIndexToId( idx, tex->channels() );
}

uint32_t RendererVK::channelCount( Texture t ) const
{
    return ( t & DATA_BITS ) >> std::countr_zero( DATA_BITS );
}

void RendererVK::beginFrame()
{
    ZoneScoped;
    if ( m_pendingVSyncChange ) {
        recreateSwapchain();
    }
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
    fr.m_state = Frame::State::eNone;
    fr.m_uniformBuffer.reset();
    fr.m_commandPool.reset();
    for ( auto& set : fr.m_descriptorSets ) {
        set.reset();
    }

    beginRecording( fr.m_cmdColorPass );
    beginRecording( fr.m_cmdDepthPrepass );
}

void RendererVK::deleteBuffer( Buffer b )
{
    ZoneScoped;
    const uint32_t bufferIndex = bufferIdToIndex( b );
    assert( bufferIndex != INVALID_INDEX );
    BufferVK* buff = m_bufferSlots[ bufferIndex ].exchange( nullptr );
    assert( buff );
    m_bufferIndexer.release( bufferIndex );
    Bottleneck bottleneck{ m_resourceDeleteBottleneck };
    m_resourceDelete.emplace_back( buff );
}

void RendererVK::deleteTexture( Texture t )
{
    ZoneScoped;
    const uint32_t textureIndex = textureIdToIndex( t );
    assert( textureIndex != INVALID_INDEX );

    TextureVK* tex = m_textureSlots[ textureIndex ].exchange( nullptr );
    assert( tex );
    m_textureIndexer.release( textureIndex );
    Bottleneck bottleneck{ m_resourceDeleteBottleneck };
    m_resourceDelete.emplace_back( tex );
}

void RendererVK::recreateSwapchain()
{
    ZoneScoped;
    vkDeviceWaitIdle( m_device );

    VSync v = m_pendingVSyncChange ? *m_pendingVSyncChange : m_swapchain.vsync();
    m_pendingVSyncChange.reset();

    m_swapchain = Swapchain( m_physicalDevice
        , m_device
        , m_surface
        , { m_queueManager.graphicsFamily(), m_queueManager.presentFamily() }
        , v
        , m_swapchain.steal()
    );
}

void RendererVK::setResolution( uint32_t width, uint32_t height )
{
    assert( width && height );
    if ( m_resolution.width == width && m_resolution.height == height ) return;
    uint64_t packedResolution = static_cast<uint64_t>( width );
    packedResolution <<= 32;
    packedResolution |= height;
    m_pendingResolutionChange.store( packedResolution );
}

void RendererVK::refreshResolution()
{
    uint64_t packedResolution = static_cast<uint64_t>( m_resolution.width );
    packedResolution <<= 32;
    packedResolution |= m_resolution.height;
    m_pendingResolutionChange.store( packedResolution );
}

void RendererVK::recreateRenderTargets( VkExtent2D resolution )
{
    ZoneScoped;
    m_resolution = resolution;
    if ( m_mainPass.m_vrs ) {
        resolution.width *= 2;
        resolution.height *= 2;
    }
    for ( auto& it : m_frames ) {
        it.m_renderDepthTarget = Image{
            m_physicalDevice
            , m_device
            , resolution
            , m_depthFormat
            , 1
            , 1
            , Image::RTGT_DEPTH.usage
            , Image::RTGT_DEPTH.memoryFlags
            , Image::RTGT_DEPTH.aspectFlags
        };
        it.m_renderTarget = Image{
            m_physicalDevice
            , m_device
            , resolution
            , m_colorFormat
            , 1
            , 1
            , Image::RTGT_COLOR.usage
            , Image::RTGT_COLOR.memoryFlags
            , Image::RTGT_COLOR.aspectFlags
        };
        it.m_renderTargetTmp = Image{
            m_physicalDevice
            , m_device
            , resolution
            , m_colorFormat
            , 1
            , 1
            , Image::RTGT_COLOR.usage
            , Image::RTGT_COLOR.memoryFlags
            , Image::RTGT_COLOR.aspectFlags
        };
    }
    vkDeviceWaitIdle( m_device );
}

static bool operator == ( const VkExtent2D& lhs, const VkExtent2D& rhs ) noexcept
{
    return lhs.width == rhs.width && lhs.height == rhs.height;
}

void RendererVK::endFrame()
{
    ZoneScoped;
    m_lastPipeline = nullptr;

    Frame& fr = m_frames[ m_currentFrame ];
    switch ( fr.m_state ) {
    case Frame::State::eGraphics:
        m_depthPrepass.end( fr.m_cmdDepthPrepass );
        m_mainPass.end( fr.m_cmdColorPass );
        break;
    case Frame::State::eCompute:
    [[unlikely]] default:
        break;
    }

    [[maybe_unused]]
    const VkResult cmdEndD = vkEndCommandBuffer( fr.m_cmdDepthPrepass );
    assert( cmdEndD == VK_SUCCESS );

    fr.m_renderTarget.transfer( fr.m_cmdColorPass, constants::copyFrom );
    transferImage( fr.m_cmdColorPass, m_swapchain.image( m_currentFrame ), constants::undefined, constants::copyTo );

    // resize if necessary
    const VkExtent2D srcExtent = fr.m_renderTarget.extent();
    const VkExtent2D dstExtent = m_swapchain.extent();
    const VkOffset3D srcOffset{ .x = (int)srcExtent.width, .y = (int)srcExtent.height, .z = 1 };
    const VkOffset3D dstOffset{ .x = (int)dstExtent.width, .y = (int)dstExtent.height, .z = 1 };
    const VkImageBlit region{
        .srcSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1 },
        .srcOffsets{ {}, srcOffset },
        .dstSubresource{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1 },
        .dstOffsets{ {}, dstOffset },
    };
    vkCmdBlitImage( fr.m_cmdColorPass
        , fr.m_renderTarget.image()
        , constants::copyFrom.m_layout
        , m_swapchain.image( m_currentFrame )
        , constants::copyTo.m_layout
        , 1
        , &region
        , srcExtent == dstExtent ? VK_FILTER_NEAREST : VK_FILTER_LINEAR
    );
    transferImage( fr.m_cmdColorPass, m_swapchain.image( m_currentFrame ), constants::copyTo, constants::present );

    [[maybe_unused]]
    const VkResult cmdEnd = vkEndCommandBuffer( fr.m_cmdColorPass );
    assert( cmdEnd == VK_SUCCESS );

    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };


    beginRecording( fr.m_cmdUniform );
    fr.m_uniformBuffer.transfer( fr.m_cmdUniform  );
    [[maybe_unused]]
    const VkResult uniformOK = vkEndCommandBuffer( fr.m_cmdUniform );
    assert( uniformOK == VK_SUCCESS );

    std::array cmds{
        fr.m_cmdUniform,
        fr.m_cmdDepthPrepass,
        fr.m_cmdColorPass,
    };

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
        auto [ queue, bottleneck ] = m_queueManager.graphics();
        assert( queue );
        assert( bottleneck );
        Bottleneck lock{ *bottleneck };
        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( queue );
    }

    decltype(m_resourceDelete) tmp{ m_resourceDelete.get_allocator() };
    {
        Bottleneck lock{ m_resourceDeleteBottleneck };
        std::swap( tmp, m_resourceDelete );
    }

    std::ranges::for_each( tmp, []( auto& a ) { std::visit( ResourceDeleter{}, a ); } );
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

    auto [ queue, bottleneck ] = m_queueManager.present();
    Bottleneck lock{ *bottleneck };
    switch ( vkQueuePresentKHR( queue, &presentInfo ) ) {
    [[likely]]
    case VK_SUCCESS: break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        recreateSwapchain();
        break;
    default:
        assert( !"failed to present" );
    }

    if ( const uint64_t packedResolution = m_pendingResolutionChange.exchange( 0 ); packedResolution ) [[unlikely]] {
        const VkExtent2D res{
            .width = static_cast<uint32_t>( packedResolution >> 32 ),
            .height = static_cast<uint32_t>( packedResolution & 0xFFFF'FFFFull ),
        };
        recreateRenderTargets( res );
    }
    vkQueueWaitIdle( queue );
}

void RendererVK::push( const PushBuffer& pushBuffer, const void* constant )
{
    assert( pushBuffer.m_pipeline < m_pipelines.size() );
    assert( pushBuffer.m_instanceCount > 0 );

    Frame& fr = m_frames[ m_currentFrame ];
    PipelineVK& currentPipeline = m_pipelines[ pushBuffer.m_pipeline ];

    switch ( fr.m_state ) {
    case Frame::State::eCompute: {
        fr.m_state = Frame::State::eGraphics;
        m_depthPrepass.resume( fr.m_cmdDepthPrepass, fr.m_renderDepthTarget, fr.m_renderTarget );
        m_mainPass.resume( fr.m_cmdColorPass, fr.m_renderDepthTarget, fr.m_renderTarget );
    } break;

    case Frame::State::eNone: {
        fr.m_state = Frame::State::eGraphics;
        m_depthPrepass.begin( fr.m_cmdDepthPrepass, fr.m_renderDepthTarget, fr.m_renderTarget );
        m_mainPass.begin( fr.m_cmdColorPass, fr.m_renderDepthTarget, fr.m_renderTarget );
    } break;
    [[likely]] default:
        break;
    }

    const bool rebindPipeline = m_lastPipeline != &currentPipeline;
    const bool depthWrite = currentPipeline.depthWrite();
    const bool updateLineWidth = currentPipeline.useLines() && pushBuffer.m_lineWidth != m_lastLineWidth;
    const bool bindBuffer = pushBuffer.m_vertexBuffer;
    uint32_t verticeCount = pushBuffer.m_verticeCount;

    auto& descriptorPool = fr.m_descriptorSets[ currentPipeline.descriptorSetId() ];

    m_lastPipeline = &currentPipeline;

    const VkDescriptorBufferInfo uniformInfo = fr.m_uniformBuffer.copy( constant, currentPipeline.pushConstantSize() );
    const VkDescriptorSet descriptorSet = descriptorPool.next();
    assert( descriptorSet != VK_NULL_HANDLE );


    if ( rebindPipeline ) {
        if ( depthWrite ) vkCmdBindPipeline( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.depthPrepass() );
        vkCmdBindPipeline( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline );
    }

    union BindInfo {
        VkDescriptorBufferInfo bufferInfo;
        VkDescriptorImageInfo imageInfo;
    };

    std::array<BindInfo, 8> bindInfo{};
    PipelineVK::DescriptorWrites descriptorWrites = currentPipeline.descriptorWrites();
    const uint32_t descriptorWriteCount = currentPipeline.descriptorWriteCount();

    for ( uint32_t i = 0; i < descriptorWriteCount; ++i ) {
        descriptorWrites[ i ].dstSet = descriptorSet;
        switch ( descriptorWrites[ i ].descriptorType ) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            bindInfo[ i ].bufferInfo = uniformInfo;
            descriptorWrites[ i ].pBufferInfo = &bindInfo[ i ].bufferInfo;
            break;

        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
            const uint32_t texIdx = textureIdToIndex( pushBuffer.m_fragmentTexture[ descriptorWrites[ i ].dstBinding ] );
            const TextureVK* texture = m_defaultTexture;
            if ( texIdx != INVALID_INDEX ) [[likely]] {
                texture = m_textureSlots[ texIdx ];
            }
            bindInfo[ i ].imageInfo = texture->imageInfo();
            descriptorWrites[ i ].pImageInfo = &bindInfo[ i ].imageInfo;
        } break;
        default:
            assert( !"here be dragons" );
            return;
        }
    }
    vkUpdateDescriptorSets( m_device, descriptorWriteCount, descriptorWrites.data(), 0, nullptr );

    if ( depthWrite ) vkCmdBindDescriptorSets( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
    vkCmdBindDescriptorSets( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );

    if ( currentPipeline.useLines() && ( updateLineWidth || rebindPipeline ) ) [[unlikely]] {
        m_lastLineWidth = pushBuffer.m_lineWidth;
        if ( depthWrite ) vkCmdSetLineWidth( fr.m_cmdDepthPrepass, pushBuffer.m_lineWidth );
        vkCmdSetLineWidth( fr.m_cmdColorPass, pushBuffer.m_lineWidth );
    }

    if ( bindBuffer ) {
        const uint32_t bufferIndex = bufferIdToIndex( pushBuffer.m_vertexBuffer );
        assert( bufferIndex != INVALID_INDEX );
        const BufferVK* b = m_bufferSlots[ bufferIndex ];
        assert( b );
        std::array<VkBuffer, 1> buffers{ *b };
        const std::array<VkDeviceSize, 1> offsets{ 0 };
        verticeCount = b->sizeInBytes() / currentPipeline.vertexStride();
        if ( depthWrite ) vkCmdBindVertexBuffers( fr.m_cmdDepthPrepass, 0, 1, buffers.data(), offsets.data() );
        vkCmdBindVertexBuffers( fr.m_cmdColorPass, 0, 1, buffers.data(), offsets.data() );
    }

    if ( depthWrite ) vkCmdDraw( fr.m_cmdDepthPrepass, verticeCount, pushBuffer.m_instanceCount, 0, 0 );
    vkCmdDraw( fr.m_cmdColorPass, verticeCount, pushBuffer.m_instanceCount, 0, 0 );
}

void RendererVK::dispatch( [[maybe_unused]] const DispatchInfo& dispatchInfo, [[maybe_unused]] const void* constant )
{
    assert( dispatchInfo.m_pipeline < m_pipelines.size() );

    Frame& fr = m_frames[ m_currentFrame ];
    PipelineVK& currentPipeline = m_pipelines[ dispatchInfo.m_pipeline ];

    switch ( fr.m_state ) {
    case Frame::State::eGraphics:
        fr.m_state = Frame::State::eCompute;
        m_depthPrepass.end( fr.m_cmdDepthPrepass );
        m_mainPass.end( fr.m_cmdColorPass );
        break;
    default:
        break;
    }

    auto& descriptorPool = fr.m_descriptorSets[ currentPipeline.descriptorSetId() ];
    const VkDescriptorBufferInfo uniformInfo = fr.m_uniformBuffer.copy( constant, currentPipeline.pushConstantSize() );
    const VkDescriptorSet descriptorSet = descriptorPool.next();
    assert( descriptorSet != VK_NULL_HANDLE );

    union BindInfo {
        VkDescriptorBufferInfo bufferInfo;
        VkDescriptorImageInfo imageInfo;
    };

    const Image* rtgt[] = {
        &fr.m_renderTarget,
        &fr.m_renderTargetTmp,
    };
    uint32_t rtgtId = 0;

    std::array<BindInfo, 32> bindInfo{};
    PipelineVK::DescriptorWrites descriptorWrites = currentPipeline.descriptorWrites();
    const uint32_t descriptorWriteCount = currentPipeline.descriptorWriteCount();
    for ( uint32_t i = 0; i < descriptorWriteCount; ++i ) {
        descriptorWrites[ i ].dstSet = descriptorSet;
        switch ( descriptorWrites[ i ].descriptorType ) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            bindInfo[ i ].bufferInfo = uniformInfo;
            descriptorWrites[ i ].pBufferInfo = &bindInfo[ i ].bufferInfo;
            break;

        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            assert( rtgtId < 2 );
            bindInfo[ i ].imageInfo = rtgt[ rtgtId++ ]->imageInfo();
            descriptorWrites[ i ].pImageInfo = &bindInfo[ i ].imageInfo;
        break;
        default:
            assert( !"here be dragons" );
            return;
        }
    }
    vkUpdateDescriptorSets( m_device, descriptorWriteCount, descriptorWrites.data(), 0, nullptr );

    fr.m_renderTarget.transfer( fr.m_cmdColorPass, constants::computeReadWrite );
    fr.m_renderTargetTmp.transfer( fr.m_cmdColorPass, constants::computeReadWrite );
    vkCmdBindDescriptorSets( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
    vkCmdBindPipeline( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline );

    const VkExtent2D extent = fr.m_renderTarget.extent();
    vkCmdDispatch( fr.m_cmdColorPass, extent.width / 4, extent.height / 4, 1 );

    std::swap( fr.m_renderTarget, fr.m_renderTargetTmp );
}
