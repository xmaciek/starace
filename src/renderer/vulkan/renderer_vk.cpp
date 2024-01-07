#include "renderer_vk.hpp"

#include "utils_vk.hpp"

#include <SDL_vulkan.h>
#include <Tracy.hpp>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>
#include <iostream>

namespace vk {
bool dllLoad();
void dllUnload();
}

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

// arbitrary values
static constexpr uint32_t INVALID_INDEX = 0xFFFF'FFFFu;
static constexpr uint32_t BUFFER_ID_CHECK = 0x10FF'0000u;
static constexpr uint32_t TEXTURE_ID_CHECK = 0x20FF'0000u;

template <uint32_t TMask>
requires ( TMask > 0 && ( TMask & 0xFFFFu ) == 0 )
static constexpr uint32_t index2Id( uint32_t index ) { return index | TMask; }

template <uint32_t TMask>
requires ( TMask > 0 && ( TMask & 0xFFFFu ) == 0 )
static uint32_t id2Index( uint32_t id )
{
    if ( ( id & ~0xFFFFu ) == TMask ) [[likely]]
        return id & 0xFFFFu;
    return INVALID_INDEX;
}

static constexpr auto& bufferIndexToId = index2Id<BUFFER_ID_CHECK>;
static constexpr auto& bufferIdToIndex = id2Index<BUFFER_ID_CHECK>;
static constexpr auto& textureIndexToId = index2Id<TEXTURE_ID_CHECK>;
static constexpr auto& textureIdToIndex = id2Index<TEXTURE_ID_CHECK>;

static Renderer* g_instance = nullptr;

static constexpr std::array REQUIRED_DEVICE_EXTENSIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

constexpr std::size_t operator ""_MiB( unsigned long long v ) noexcept
{
    return v << 20;
}

template <typename T>
struct Wishlist {
    std::pmr::vector<T>* m_checklist = nullptr;
    std::pmr::vector<const char*>* m_ret = nullptr;

    static bool scmp( const VkLayerProperties& prop, const char* name ) { return std::strcmp( prop.layerName, name ) == 0; }
    static bool scmp( const VkExtensionProperties& prop, const char* name ) { return std::strcmp( prop.extensionName, name ) == 0; }

    void operator () ( const char* name )
    {
        auto cmp = [name]( const auto& prop ) { return scmp( prop, name ); };
        if ( std::find_if( m_checklist->begin(), m_checklist->end(), cmp ) == m_checklist->end() ) return;
        m_ret->emplace_back( name );
    }
};

static std::pmr::vector<const char*> enabledLayers()
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties( &count, nullptr );
    std::pmr::vector<VkLayerProperties> layerList( count );
    vkEnumerateInstanceLayerProperties( &count, layerList.data() );

    std::pmr::vector<const char*> ret;
    [[maybe_unused]]
    Wishlist<VkLayerProperties> wishlist{ &layerList, &ret };

#if ENABLE_VULKAN_VALIDATION
    wishlist( "VK_LAYER_KHRONOS_validation" );
    // wishlist( "VK_LAYER_RENDERDOC_Capture" );
#endif
    return ret;
}

static std::pmr::vector<const char*> enabledExtensions( SDL_Window* window )
{
    uint32_t count = 0;
    SDL_Vulkan_GetInstanceExtensions( window, &count, nullptr );
    std::pmr::vector<const char*> ret( count );
    SDL_Vulkan_GetInstanceExtensions( window, &count, ret.data() );

    vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
    std::pmr::vector<VkExtensionProperties> extensionList( count );
    vkEnumerateInstanceExtensionProperties( nullptr, &count, extensionList.data() );

    [[maybe_unused]]
    Wishlist<VkExtensionProperties> wishlist{ &extensionList, &ret };
#if ENABLE_VULKAN_VALIDATION
    wishlist( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif
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
: m_unloader{ .fn = &vk::dllUnload }
, m_window( createInfo.window )
{
    ZoneScoped;
    assert( !g_instance );
    g_instance = this;

    if ( !vk::dllLoad() ) {
        assert( !"Failed to load vulkan library" );
        std::cout << "Failed to load vulkan library" << std::endl;
        std::abort();
    }

    const std::pmr::vector<const char*> layers = enabledLayers();
    const std::pmr::vector<const char*> extensions = enabledExtensions( m_window );

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


        const VkInstanceCreateInfo instanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>( layers.size() ),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>( extensions.size() ),
            .ppEnabledExtensionNames = extensions.data(),
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
            .samplerAnisotropy = VK_TRUE,
        };
        const VkDeviceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = std::size( queueCreateInfo ),
            .pQueueCreateInfos = queueCreateInfo,
            .enabledLayerCount = static_cast<uint32_t>( layers.size() ),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>( REQUIRED_DEVICE_EXTENSIONS.size() ),
            .ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENSIONS.data(),
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
        , createInfo.vsync
    );

    vkGetDeviceQueue( m_device, m_queueFamilyPresent, 0, &m_queuePresent );

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

    m_mainPass = RenderPass{ m_device, m_colorFormat, m_depthFormat };
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

    m_transferCommandPool = CommandPool{ m_device, 1, m_queueFamilyGraphics };
    m_transferCmd = m_transferCommandPool[ 0 ];

    m_frames.resize( m_swapchain.imageCount() );

    for ( auto& it : m_frames ) {
        it.m_commandPool = CommandPool{ m_device, 3, m_queueFamilyGraphics };
        it.m_uniformBuffer = Uniform{ m_physicalDevice, m_device, 2_MiB, physicalProperties.limits.minUniformBufferOffsetAlignment };
        it.m_cmdTransfer = it.m_commandPool[ 0 ];
        it.m_cmdDepthPrepass = it.m_commandPool[ 1 ];
        it.m_cmdRender = it.m_commandPool[ 2 ];
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

    static_assert( sizeof( pci.m_binding ) == sizeof( uint64_t ), "TODO: different method of matching descriptor sets" );
    uint64_t binding = 0;
    std::memcpy( &binding, pci.m_binding.data(), sizeof( uint64_t ) );
    auto [ descriptorId, add ] = findOrAddDescriptorId( m_pipelineDescriptorIds, binding );
    if ( add ) {
        for ( auto& fr : m_frames ) {
            fr.m_descriptorSets[ descriptorId ] = DescriptorSet{ m_device, pci.m_binding };
        }
    }

    DescriptorSet* descriptorSet = &m_frames[ 0 ].m_descriptorSets[ descriptorId ];
    if ( pci.m_computeShader ) {
        m_pipelines[ slot ] = PipelineVK{
            pci
            , m_device
            , descriptorSet->layout()
            , descriptorId
        };
        return slot;
    }

    m_pipelines[ slot ] = PipelineVK{
        pci
        , m_device
        , m_mainPass
        , m_depthPrepass
        , descriptorSet->layout()
        , descriptorId
    };
    return slot;
}

RendererVK::~RendererVK()
{
    ZoneScoped;
    for ( auto& it : m_frames ) {
        it = {};
    }
    m_transferCommandPool = {};

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

void RendererVK::setVSync( VSync v )
{
    m_pendingVSyncChange = v;
}

bool RendererVK::supportedVSync( VSync v ) const
{
    assert( m_physicalDevice );
    assert( m_surface );
    auto vsyncs = Swapchain::supportedVSyncs( m_physicalDevice, m_surface );
    assert( static_cast<uint32_t>( v ) < vsyncs.size() );
    return vsyncs[ static_cast<uint32_t>( v ) ];
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

VkCommandBuffer RendererVK::flushUniforms()
{
    ZoneScoped;

    Frame& fr = m_frames[ m_currentFrame ];
    VkCommandBuffer cmd = fr.m_cmdTransfer;
    beginRecording( cmd );
    fr.m_uniformBuffer.transfer( cmd );

    [[maybe_unused]]
    const VkResult endOK = vkEndCommandBuffer( cmd );
    assert( endOK == VK_SUCCESS );
    return cmd;

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
        auto [ queue, bottleneck ] = m_queueTransfer;
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

Buffer RendererVK::createBuffer( std::pmr::vector<float>&& vec )
{
    return createBuffer( std::span<const float>{ vec.data(), vec.data() + vec.size() } );
}

std::pmr::memory_resource* RendererVK::allocator()
{
    return std::pmr::get_default_resource();
}

Texture RendererVK::createTexture( const TextureCreateInfo& tci, std::span<const uint8_t> data )
{
    ZoneScoped;
    assert( tci.width > 0 );
    assert( tci.height > 0 );
    assert( !data.empty() );

    const uint32_t size = static_cast<uint32_t>( data.size() );

    BufferVK staging{ m_physicalDevice, m_device, BufferVK::STAGING, size };
    staging.copyData( data.data() + tci.dataBeginOffset );

    TextureVK* tex = new TextureVK{ tci, m_physicalDevice, m_device };

    VkCommandBuffer cmd = m_transferCmd;
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    {
        ZoneScopedN( "queue submit" );
        auto [ queue, bottleneck ] = m_queueTransfer;
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

    return textureIndexToId( idx );
}

Texture RendererVK::createTexture( const TextureCreateInfo& tci, std::pmr::vector<uint8_t>&& data )
{
    return createTexture( tci, static_cast<std::span<const uint8_t>>( data ) );
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

    beginRecording( fr.m_cmdRender );
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
    Bottleneck bottleneck{ m_bufferBottleneck };
    m_bufferPendingDelete.push_back( buff );
}

void RendererVK::deleteTexture( Texture t )
{
    ZoneScoped;
    const uint32_t textureIndex = textureIdToIndex( t );
    assert( textureIndex != INVALID_INDEX );

    TextureVK* tex = m_textureSlots[ textureIndex ].exchange( nullptr );
    assert( tex );
    m_textureIndexer.release( textureIndex );
    Bottleneck bottleneck{ m_textureBottleneck };
    m_texturePendingDelete.push_back( tex );
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
        , { m_queueFamilyGraphics, m_queueFamilyPresent }
        , v
        , m_swapchain.steal()
    );
}

void RendererVK::setResolution( uint32_t width, uint32_t height )
{
    uint64_t packedResolution = static_cast<uint64_t>( width );
    packedResolution <<= 32;
    packedResolution |= height;
    m_pendingResolutionChange.store( packedResolution );
}

void RendererVK::recreateRenderTargets( const VkExtent2D& resolution )
{
    ZoneScoped;
    for ( auto& it : m_frames ) {
        it.m_renderDepthTarget = RenderTarget{
            RenderTarget::DEPTH
            , m_physicalDevice
            , m_device
            , m_depthPrepass
            , resolution
            , m_depthFormat
            , nullptr
        };
        it.m_renderTarget = RenderTarget{
            RenderTarget::COLOR
            , m_physicalDevice
            , m_device
            , m_mainPass
            , resolution
            , m_colorFormat
            , it.m_renderDepthTarget.view()
        };
        it.m_renderTargetTmp = RenderTarget{
            RenderTarget::COLOR
            , m_physicalDevice
            , m_device
            , m_mainPass
            , resolution
            , m_colorFormat
            , it.m_renderDepthTarget.view()
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
        m_mainPass.end( fr.m_cmdRender );
        break;
    case Frame::State::eCompute:
    [[unlikely]] default:
        break;
    }

    [[maybe_unused]]
    const VkResult cmdEndD = vkEndCommandBuffer( fr.m_cmdDepthPrepass );
    assert( cmdEndD == VK_SUCCESS );

    VkCommandBuffer cmd = fr.m_cmdRender;
    fr.m_renderTarget.transfer( cmd, constants::copyFrom );
    transferImage( cmd, m_swapchain.image( m_currentFrame ), constants::undefined, constants::copyTo );

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
    vkCmdBlitImage( cmd
        , fr.m_renderTarget.image()
        , constants::copyFrom.m_layout
        , m_swapchain.image( m_currentFrame )
        , constants::copyTo.m_layout
        , 1
        , &region
        , srcExtent == dstExtent ? VK_FILTER_NEAREST : VK_FILTER_LINEAR
    );
    transferImage( cmd, m_swapchain.image( m_currentFrame ), constants::copyTo, constants::present );

    [[maybe_unused]]
    const VkResult cmdEnd = vkEndCommandBuffer( cmd );
    assert( cmdEnd == VK_SUCCESS );

    VkSemaphore renderSemaphores[]{ m_semaphoreRender };
    VkPipelineStageFlags waitStages[]{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    std::array cmds{
        flushUniforms()
        , fr.m_cmdDepthPrepass
        , cmd
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
        auto [ queue, bottleneck ] = m_queueGraphics;
        assert( queue );
        assert( bottleneck );
        Bottleneck lock{ *bottleneck };
        [[maybe_unused]]
        const VkResult submitOK = vkQueueSubmit( queue, 1, &submitInfo, VK_NULL_HANDLE );
        assert( submitOK == VK_SUCCESS );
        vkQueueWaitIdle( queue );
    }

    auto release = []( auto& vec, auto& bottleneck )
    {
        std::remove_reference_t<decltype(vec)> tmp{ vec.get_allocator() };
        {
            Bottleneck lock{ bottleneck };
            std::swap( tmp, vec );
        }
        for ( auto* it : tmp ) {
            assert( it );
            delete it;
        }
    };

    release( m_bufferPendingDelete, m_bufferBottleneck );
    release( m_texturePendingDelete, m_textureBottleneck );
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
    [[likely]]
    case VK_SUCCESS: break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        recreateSwapchain();
        break;
    default:
        assert( !"failed to present" );
    }

    do {
        const uint64_t packedResolution = m_pendingResolutionChange.exchange( 0 );
        if ( packedResolution == 0 ) [[likely]] break;
        const VkExtent2D res{
            .width = static_cast<uint32_t>( packedResolution >> 32 ),
            .height = static_cast<uint32_t>( packedResolution & 0xFFFF'FFFFull ),
        };
        assert( res.width && res.height );
        const VkExtent2D currentRes = m_frames[ 0 ].m_renderTarget.extent();
        if ( currentRes == res ) break;
        recreateRenderTargets( res );
    } while ( 0 );
    vkQueueWaitIdle( m_queuePresent );
}

void RendererVK::push( const PushBuffer& pushBuffer, const void* constant )
{
    assert( pushBuffer.m_pipeline < m_pipelines.size() );
    assert( pushBuffer.m_instanceCount > 0 );

    Frame& fr = m_frames[ m_currentFrame ];
    PipelineVK& currentPipeline = m_pipelines[ pushBuffer.m_pipeline ];

    fr.m_renderTarget.transfer( fr.m_cmdRender, constants::fragmentWrite );
    switch ( fr.m_state ) {
    case Frame::State::eCompute: {
        fr.m_state = Frame::State::eGraphics;
        const VkRect2D rect{ .extent = fr.m_renderDepthTarget.extent() };
        m_depthPrepass.resume( fr.m_cmdDepthPrepass, fr.m_renderDepthTarget.framebuffer(), rect );
        m_mainPass.resume( fr.m_cmdRender, fr.m_renderTarget.framebuffer(), rect );
    } break;

    case Frame::State::eNone: {
        fr.m_state = Frame::State::eGraphics;
        const VkRect2D rect{ .extent = fr.m_renderDepthTarget.extent() };
        m_depthPrepass.begin( fr.m_cmdDepthPrepass, fr.m_renderDepthTarget.framebuffer(), rect );
        m_mainPass.begin( fr.m_cmdRender, fr.m_renderTarget.framebuffer(), rect );
    } break;
    default:
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
    VkCommandBuffer cmd = fr.m_cmdRender;


    if ( rebindPipeline ) {
        if ( depthWrite ) vkCmdBindPipeline( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.depthPrepass() );
        vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline );
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
            const uint32_t texIdx = textureIdToIndex( pushBuffer.m_resource[ i ].texture );
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
    vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );

    if ( updateLineWidth ) {
        m_lastLineWidth = pushBuffer.m_lineWidth;
        if ( depthWrite ) vkCmdSetLineWidth( fr.m_cmdDepthPrepass, pushBuffer.m_lineWidth );
        vkCmdSetLineWidth( cmd, pushBuffer.m_lineWidth );
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
        vkCmdBindVertexBuffers( cmd, 0, 1, buffers.data(), offsets.data() );
    }

    if ( depthWrite ) vkCmdDraw( fr.m_cmdDepthPrepass, verticeCount, pushBuffer.m_instanceCount, 0, 0 );
    vkCmdDraw( cmd, verticeCount, pushBuffer.m_instanceCount, 0, 0 );
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
        m_mainPass.end( fr.m_cmdRender );
        break;
    default:
        break;
    }

    VkCommandBuffer cmd = fr.m_cmdRender;
    auto& descriptorPool = fr.m_descriptorSets[ currentPipeline.descriptorSetId() ];
    const VkDescriptorBufferInfo uniformInfo = fr.m_uniformBuffer.copy( constant, currentPipeline.pushConstantSize() );
    const VkDescriptorSet descriptorSet = descriptorPool.next();
    assert( descriptorSet != VK_NULL_HANDLE );

    union BindInfo {
        VkDescriptorBufferInfo bufferInfo;
        VkDescriptorImageInfo imageInfo;
    };

    const RenderTarget* rtgt[] = {
        &fr.m_renderTarget,
        &fr.m_renderTargetTmp,
    };
    uint32_t rtgtId = 0;

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

    fr.m_renderTarget.transfer( cmd, constants::computeReadWrite );
    fr.m_renderTargetTmp.transfer( cmd, constants::computeReadWrite );
    vkCmdBindDescriptorSets( cmd, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
    vkCmdBindPipeline( cmd, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline );

    const VkExtent2D extent = fr.m_renderTarget.extent();
    vkCmdDispatch( cmd, extent.width / 8, extent.height / 8, 1 );

    std::swap( fr.m_renderTarget, fr.m_renderTargetTmp );
}
