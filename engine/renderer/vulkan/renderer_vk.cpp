#include "renderer_vk.hpp"

#include "utils_vk.hpp"

#include <platform/utils.hpp>

#include <SDL_vulkan.h>
#include <profiler.hpp>

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>


SDL_WindowFlags Renderer::windowFlag = SDL_WINDOW_VULKAN;
Renderer::FNCreate* Renderer::create = []( const Renderer::CreateInfo& ci ) -> Renderer*
{
    return new RendererVK{ ci };
};
Renderer* Renderer::s_instance = nullptr;

struct ResourceDeleter {
    void operator () ( RendererVK::ResourceDelete& r ) { r.visit( *this ); }
    void operator () ( TextureVK*& t ) { delete std::exchange( t, nullptr ); }
    void operator () ( std::atomic<TextureVK*>& t ) { delete t.exchange( nullptr ); }
    void operator () ( BufferVK*& b ) { delete std::exchange( b, nullptr ); }
    void operator () ( std::atomic<BufferVK*>& b ) { delete b.exchange( nullptr ); }
};

struct TextureExtra {
    enum : uint8_t { SENTINEL = 0x2F };
    uint16_t index = 0;
    uint8_t channelCount = 0;
    uint8_t sentinel = SENTINEL;

    operator Texture () const { return std::bit_cast<Texture>( *this ); }
    operator bool () const
    {
        return sentinel == SENTINEL
            && channelCount <= 4
            && index < RendererVK::MAX_TEXTURES
            ;
    }
};

struct BufferExtra {
    enum : uint8_t { SENTINEL = 0x1F };
    uint16_t index = 0;
    uint8_t padding = 0;
    uint8_t sentinel = SENTINEL;

    operator Buffer () const { return std::bit_cast<Buffer>( *this ); }
    operator bool () const
    {
        return sentinel == SENTINEL
            && index < RendererVK::MAX_BUFFERS
            ;
    }
};

constexpr std::size_t operator ""_MiB( unsigned long long v ) noexcept
{
    return v << 20;
}

static std::pmr::vector<const char*> windowExtensions()
{
    uint32_t count = 0;
    auto ptr = SDL_Vulkan_GetInstanceExtensions( &count );
    std::pmr::vector<const char*> ret( count );
    std::copy_n( ptr, count, ret.data() );
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
        auto it = std::ranges::find_if( table, FormatSupportTest{ physicalDevice } );
        assert( it != table.end() );
        assert( it->format != VK_FORMAT_UNDEFINED );
        return it->format;
    };
};

Surface::~Surface()
{
    if ( m_surface ) SDL_Vulkan_DestroySurface( m_instance, m_surface, nullptr );
}

Surface::Surface( SDL_Window* window, VkInstance inst )
: m_instance( inst )
{
    if ( !SDL_Vulkan_CreateSurface( window, inst, nullptr, &m_surface ) ) {
        platform::showFatalError( "Vulkan error", "Failed to create SDL Vulkan surface" );
    }
}

Surface::Surface( Surface&& r )
: m_surface{ std::exchange( r.m_surface, {} ) }
, m_instance{ std::exchange( r.m_instance, {} ) }
{}

Surface& Surface::operator = ( Surface&& r )
{
    std::swap( m_surface, r.m_surface );
    std::swap( m_instance, r.m_instance );
    return *this;
}

Semaphore::~Semaphore()
{
    destroy<vkDestroySemaphore, VkSemaphore>( m_device, m_semaphore );
}

Semaphore::Semaphore( VkDevice device )
: m_device{ device }
{
    static constexpr VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    [[maybe_unused]]
    const VkResult createOK = vkCreateSemaphore( m_device, &semaphoreInfo, nullptr, &m_semaphore );
    assert( createOK == VK_SUCCESS );
}

Semaphore::Semaphore( Semaphore&& r )
: m_semaphore{ std::exchange( r.m_semaphore, {} ) }
, m_device{ std::exchange( r.m_device, {} ) }
{}

Semaphore& Semaphore::operator = ( Semaphore&& r )
{
    std::swap( m_semaphore, r.m_semaphore );
    std::swap( m_device, r.m_device );
    return *this;
}

RendererVK::RendererVK( const Renderer::CreateInfo& createInfo )
: m_window{ createInfo.window }
{
    ZoneScoped;

    assert( !Renderer::s_instance );
    Renderer::s_instance = this;
    m_instance = Instance{ createInfo, windowExtensions() };
    m_surface = Surface{ m_window, m_instance };
    m_debugMsg = DebugMsg{ m_instance };

    VkPhysicalDeviceProperties physicalProperties{};
    m_physicalDevice = selectPhysicalDevice( m_instance, &physicalProperties );
    assert( m_physicalDevice );

    {
        using Info = FormatSupportTest::Info;

        static constexpr std::array colorFormatWishlist{
            Info{ VK_FORMAT_B10G11R11_UFLOAT_PACK32,  VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
            // NOTE: false positive validation error, in compute shader the operations will CONVERT the data, but the validation will not let me do that
            // https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Image_formats
            // Info{ VK_FORMAT_R16G16B16A16_UNORM,       VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
            // Info{ VK_FORMAT_B8G8R8A8_UNORM,           VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
            // Info{ VK_FORMAT_R8G8B8A8_UNORM,           VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT },
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
    m_device = Device{ m_physicalDevice, m_queueManager.createInfo() };
    m_queueManager.acquire( m_device );

    m_swapchain = Swapchain( m_window
        , m_physicalDevice
        , m_device
        , m_surface
        , { m_queueManager.graphicsFamily(), m_queueManager.presentFamily() }
        , createInfo.vsync
    );

    m_semaphoreAvailableImage = Semaphore{ m_device };
    m_semaphoreRender = Semaphore{ m_device };
    m_transferCommandPool = CommandPool{ m_device, 1, m_queueManager.transferFamily() };
    m_transferCmd = m_transferCommandPool[ 0 ];

    m_frames.resize( m_swapchain.imageCount() );

    for ( auto& it : m_frames ) {
        it.m_uniformBuffer = UniformVK{ m_physicalDevice, m_device, 2_MiB, physicalProperties.limits.minUniformBufferOffsetAlignment };
        it.m_commandPool = CommandPool{ m_device, 3, m_queueManager.graphicsFamily() };
        it.m_cmdUniform = it.m_commandPool[ 0 ];
        it.m_cmdDepthPrepass = it.m_commandPool[ 1 ];
        it.m_cmdColorPass = it.m_commandPool[ 2 ];
        it.m_mainPass = RenderPass{ m_device, RenderPass::eColor };
        it.m_depthPrepass = RenderPass{ m_device, RenderPass::eDepth };
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
        m_defaultTexture = m_textureSlots[ std::bit_cast<TextureExtra>( m_defaultTextureId ).index ];
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

    uint64_t hash = DescriptorSet::createBindingHash( pci );
    auto [ descriptorId, add ] = findOrAddDescriptorId( m_pipelineDescriptorIds, hash );
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
    return slot + 1;
}

RendererVK::~RendererVK()
{
    ZoneScoped;
    std::ranges::for_each( m_textureSlots, ResourceDeleter{} );
    std::ranges::for_each( m_bufferSlots, ResourceDeleter{} );
    std::ranges::for_each( m_resourceDelete, ResourceDeleter{} );
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
        for ( auto&& fr : m_frames ) {
            fr.m_mainPass.enableVRS( featureAvailable( f ) && b );
        }
        refreshResolution();
        break;
    default:
        assert( !"unhandled enum" );
        break;
    }
}

void RendererVK::pushScissor( math::vec4 rect, math::vec2 viewportExtent )
{
    Frame& fr = m_frames[ m_currentFrame ];
    auto rtgtExtent = fr.m_renderTarget.extent();
    math::vec2 aspect = math::vec2{ rtgtExtent.width, rtgtExtent.height } / viewportExtent;
    auto aspectedRect = rect * math::vec4{ aspect, aspect };
    VkRect2D scissorRect{
        VkOffset2D{
            static_cast<int32_t>( aspectedRect.x ),
            static_cast<int32_t>( aspectedRect.y ) }
        , VkExtent2D{
            static_cast<uint32_t>( aspectedRect.z ),
            static_cast<uint32_t>( aspectedRect.w )
        }
    };
    fr.pushScissor( scissorRect );
}

void RendererVK::popScissor()
{
    m_frames[ m_currentFrame ].popScissor();
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

Buffer RendererVK::createBuffer( std::span<const uint8_t> data )
{
    ZoneScoped;
    const uint32_t size = static_cast<uint32_t>( data.size() );
    BufferVK staging = getStagingBuffer( size );
    assert( staging.sizeInBytes() >= data.size() );
    staging.copyData( data );

    BufferVK* buff = new BufferVK{ m_physicalDevice, m_device, BufferVK::DEVICE_LOCAL, size };

    const VkBufferCopy copyRegion{
        .size = size,
    };

    VkCommandBuffer cmd = m_transferCmd;
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    m_transferCommandPool.reset();
    beginRecording( cmd );
    vkCmdCopyBuffer( cmd, staging, *buff, 1, &copyRegion );
    vkEndCommandBuffer( cmd );

    [[maybe_unused]]
    const VkResult submitOK = m_queueManager.submitTransfer( submitInfo );
    assert( submitOK == VK_SUCCESS );

    releaseStagingBuffer( std::move( staging ) );

    const uint32_t idx = static_cast<uint32_t>( m_bufferIndexer.next() );
    [[maybe_unused]]
    BufferVK* oldBuff = m_bufferSlots[ idx ].exchange( buff );
    assert( !oldBuff );
    return BufferExtra{ .index = (uint16_t)idx };
}

Texture RendererVK::createTexture( const TextureCreateInfo& tci, std::span<const uint8_t> data )
{
    ZoneScoped;
    assert( tci.width > 0 );
    assert( tci.height > 0 );
    assert( !data.empty() );

    const uint32_t size = static_cast<uint32_t>( data.size() );

    BufferVK staging = getStagingBuffer( size );
    assert( staging.sizeInBytes() >= size );
    staging.copyData( data );

    TextureVK* tex = new TextureVK{ tci, m_physicalDevice, m_device };

    VkCommandBuffer cmd = m_transferCmd;
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    m_transferCommandPool.reset();
    beginRecording( cmd );
    tex->transferFrom( cmd, staging, tci.mip0ByteCount );
    vkEndCommandBuffer( cmd );

    [[maybe_unused]]
    const VkResult submitOK = m_queueManager.submitTransfer( submitInfo );
    assert( submitOK == VK_SUCCESS );

    const uint32_t idx = static_cast<uint32_t>( m_textureIndexer.next() );
    [[maybe_unused]]
    TextureVK* oldTex = m_textureSlots[ idx ].exchange( tex );
    assert( !oldTex );

    releaseStagingBuffer( std::move( staging ) );
    return TextureExtra{ .index = (uint16_t)idx, .channelCount = (uint8_t)tex->channels(), };
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
        break;
    case VK_ERROR_OUT_OF_DATE_KHR:
    case VK_SUBOPTIMAL_KHR:
        recreateSwapchain();
        return beginFrame();
    default:
        assert( !"unhandled error from vkAcquireNextImageKHR" );
        break;
    }

    m_currentFrame = imageIndex;
    Frame& fr = m_frames[ imageIndex ];
    fr.reset();

    beginRecording( fr.m_cmdColorPass );
    beginRecording( fr.m_cmdDepthPrepass );
}

void RendererVK::deleteBuffer( Buffer b )
{
    ZoneScoped;
    auto buf = std::bit_cast<BufferExtra>( b );
    assert( buf );
    BufferVK* ptr = m_bufferSlots[ buf.index ].exchange( nullptr );
    assert( ptr );
    m_bufferIndexer.release( buf.index );
    Bottleneck bottleneck{ m_resourceDeleteBottleneck };
    m_resourceDelete.emplace_back( ptr );
}

void RendererVK::deleteTexture( Texture t )
{
    ZoneScoped;
    auto tex = std::bit_cast<TextureExtra>( t );
    assert( tex );

    TextureVK* ptr = m_textureSlots[ tex.index ].exchange( nullptr );
    assert( ptr );
    m_textureIndexer.release( tex.index );
    Bottleneck bottleneck{ m_resourceDeleteBottleneck };
    m_resourceDelete.emplace_back( ptr );
}

BufferVK RendererVK::getStagingBuffer( uint32_t size )
{
    ZoneScoped;
    std::array<uint32_t, STAGING_BUFFER_CACHE_COUNT> score{};
    {
        Bottleneck bottleneck{ m_stagingBuffersCacheBottleneck };
        std::ranges::transform( m_stagingBuffersCache, score.begin(), [size]( const auto& buff )
        {
            if ( buff.sizeInBytes() < size ) return 0u;
            static_assert( std::is_same_v<decltype( buff.sizeInBytes() ), uint32_t> );
            return 0xFFFFFFFFu - buff.sizeInBytes();
        } );
        auto it = std::ranges::max_element( score );
        assert( it != score.end() );
        if ( *it != 0u ) {
            auto index = static_cast<uint32_t>( std::distance( score.begin(), it ) );
            assert( index < m_stagingBuffersCache.size() );
            return std::exchange( m_stagingBuffersCache[ index ], {} );
        }
    }
    return BufferVK{ m_physicalDevice, m_device, BufferVK::STAGING, size };
}

void RendererVK::releaseStagingBuffer( BufferVK&& buffer )
{
    ZoneScoped;
    auto cmp = []( const auto& lhs, const auto& rhs ) { return lhs.sizeInBytes() < rhs.sizeInBytes(); };
    Bottleneck bottleneck{ m_stagingBuffersCacheBottleneck };
    auto it = std::ranges::min_element( m_stagingBuffersCache, cmp );
    assert( it != m_stagingBuffersCache.end() );
    if ( it->sizeInBytes() >= buffer.sizeInBytes() ) return;
    *it = std::move( buffer );
}

void RendererVK::recreateSwapchain()
{
    ZoneScoped;
    vkDeviceWaitIdle( m_device );

    VSync v = m_pendingVSyncChange ? *m_pendingVSyncChange : m_swapchain.vsync();
    m_pendingVSyncChange.reset();

    m_swapchain = Swapchain( m_window
        , m_physicalDevice
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
    if ( m_frames[ 0 ].m_mainPass.m_vrs ) {
        resolution.width *= 2;
        resolution.height *= 2;
    }

    auto cmd = m_frames[ 0 ].m_cmdColorPass;
    beginRecording( cmd );
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
        it.m_renderDepthTarget.transfer( cmd, constants::depthRead );

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
        it.m_renderTarget.transfer( cmd, constants::copyFrom );

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
        it.m_renderTargetTmp.transfer( cmd, constants::copyFrom );
    }

    [[maybe_unused]]
    const VkResult cmdEndD = vkEndCommandBuffer( cmd );
    assert( cmdEndD == VK_SUCCESS );
    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 0,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };
    [[maybe_unused]]
    const VkResult submitOK = m_queueManager.submitGraphics( submitInfo );
    assert( submitOK == VK_SUCCESS );

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
    fr.switchState( Frame::State::eNone );

    [[maybe_unused]]
    const VkResult cmdEndD = vkEndCommandBuffer( fr.m_cmdDepthPrepass );
    assert( cmdEndD == VK_SUCCESS );

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
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_semaphoreAvailableImage,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = cmds.size(),
        .pCommandBuffers = cmds.data(),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = m_semaphoreRender,
    };

    m_queueManager.submitGraphics( submitInfo );

    decltype(m_resourceDelete) tmp{ m_resourceDelete.get_allocator() };
    {
        Bottleneck lock{ m_resourceDeleteBottleneck };
        std::swap( tmp, m_resourceDelete );
    }

    std::ranges::for_each( tmp, ResourceDeleter{} );
}

void RendererVK::present()
{
    ZoneScoped;
    VkSwapchainKHR swapchain[] = { m_swapchain };
    const VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_semaphoreRender,
        .swapchainCount = 1,
        .pSwapchains = swapchain,
        .pImageIndices = &m_currentFrame,
    };

    switch ( m_queueManager.submitPresent( presentInfo ) ) {
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
}

void RendererVK::render( const RenderInfo& ri )
{
    assert( ri.m_pipeline );
    assert( ri.m_pipeline < m_pipelines.size() );
    assert( ri.m_instanceCount > 0 );

    Frame& fr = m_frames[ m_currentFrame ];
    fr.switchState( Frame::State::eGraphics );
    fr.processPendingOperations();

    PipelineVK& currentPipeline = m_pipelines[ ri.m_pipeline - 1 ];

    const bool rebindPipeline = m_lastPipeline != &currentPipeline;
    const bool depthWrite = currentPipeline.depthWrite();
    const bool updateLineWidth = currentPipeline.useLines() && ri.m_lineWidth != m_lastLineWidth;
    uint32_t verticeCount = ri.m_verticeCount;

    enum : uint32_t {
        fDepth = 0b1,
        fIndexed = 0b10,
    };
    uint32_t cmd = depthWrite ? fDepth : 0;
    auto& descriptorPool = fr.m_descriptorSets[ currentPipeline.descriptorSetPoolId() ];

    m_lastPipeline = &currentPipeline;

    const VkDescriptorSet descriptorSet = descriptorPool.next();
    assert( descriptorSet != VK_NULL_HANDLE );


    if ( rebindPipeline ) {
        if ( depthWrite ) vkCmdBindPipeline( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.depthPrepass() );
        vkCmdBindPipeline( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline );
    }


    auto pushUniforms = [&fr]( const Uniform& u )
    {
        return fr.m_uniformBuffer.copy( u.ptr, u.size );
    };

    auto findTexture = [this]( Texture t )
    {
        const auto tex = std::bit_cast<TextureExtra>( t );
        if ( tex ) [[likely]] {
            return m_textureSlots[ tex.index ].load()->imageInfo();
        }
        return m_defaultTexture->imageInfo();
    };

    auto findBuffer = [this]( Buffer buf )
    {
        const auto buffer = std::bit_cast<BufferExtra>( buf );
        assert( buffer );
        const BufferVK* b = m_bufferSlots[ buffer.index ];
        assert( b );
        return b->bindInfo();
    };

    std::array<VkDescriptorBufferInfo, RenderInfo::MAX_UNIFORMS + 1> uniformInfo{};
    int uidx = 0;
    if ( ri.m_uniform.ptr ) {
        uniformInfo[ 0 ] = pushUniforms( ri.m_uniform );
        uidx++;
    }
    std::ranges::transform( ri.m_uniforms, uniformInfo.begin() + uidx, pushUniforms );

    std::array<VkDescriptorImageInfo, RenderInfo::MAX_TEXTURES> imageInfo;
    imageInfo.fill( m_defaultTexture->imageInfo() );
    std::ranges::transform( ri.m_fragmentTexture, imageInfo.begin(), findTexture );

    std::array<VkDescriptorBufferInfo, RenderInfo::MAX_STORAGES> storageInfo{};
    std::ranges::transform( ri.m_storageBuffers, storageInfo.begin(), findBuffer );

    descriptorPool.update( descriptorSet, uniformInfo, imageInfo, storageInfo );

    if ( currentPipeline.useLines() && ( updateLineWidth || rebindPipeline ) ) [[unlikely]] {
        m_lastLineWidth = ri.m_lineWidth;
        if ( depthWrite ) vkCmdSetLineWidth( fr.m_cmdDepthPrepass, ri.m_lineWidth );
        vkCmdSetLineWidth( fr.m_cmdColorPass, ri.m_lineWidth );
    }

    auto getBuffer = [this]( Buffer buf ) -> BufferVK*
    {
        if ( !buf ) return nullptr;
        const auto buffer = std::bit_cast<BufferExtra>( buf );
        assert( buffer );
        BufferVK* b = m_bufferSlots[ buffer.index ];
        assert( b );
        return b;
    };
    if ( auto b = getBuffer( ri.m_vertexBuffer ) ) {
        if ( depthWrite ) b->bind( fr.m_cmdDepthPrepass );
        b->bind( fr.m_cmdColorPass );
        verticeCount = b->sizeInBytes() / currentPipeline.vertexStride();

        if ( auto bi = getBuffer( ri.m_indexBuffer ) ) {
            if ( depthWrite ) bi->bindAsIndex( fr.m_cmdDepthPrepass );
            bi->bindAsIndex( fr.m_cmdColorPass );
            verticeCount = b->sizeInBytes() / sizeof( uint16_t );
            cmd |= fIndexed;
        }
    }

    assert( verticeCount );
    switch ( cmd ) {
    case fDepth:
        vkCmdBindDescriptorSets( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
        vkCmdDraw( fr.m_cmdDepthPrepass, verticeCount, ri.m_instanceCount, 0, 0 );
        [[fallthrough]];

    case 0:
        vkCmdBindDescriptorSets( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
        vkCmdDraw( fr.m_cmdColorPass, verticeCount, ri.m_instanceCount, 0, 0 );
        break;

    case fDepth | fIndexed:
        vkCmdBindDescriptorSets( fr.m_cmdDepthPrepass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
        vkCmdDrawIndexed( fr.m_cmdDepthPrepass, verticeCount, ri.m_instanceCount, 0, 0, 0 );
         [[fallthrough]];

    case fIndexed:
        vkCmdBindDescriptorSets( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
        vkCmdDrawIndexed( fr.m_cmdColorPass, verticeCount, ri.m_instanceCount, 0, 0, 0 );
        break;
    }
}

void RendererVK::dispatch( const DispatchInfo& dispatchInfo )
{
    assert( dispatchInfo.m_pipeline );
    assert( dispatchInfo.m_pipeline < m_pipelines.size() );

    Frame& fr = m_frames[ m_currentFrame ];
    fr.switchState( Frame::State::eCompute );
    PipelineVK& currentPipeline = m_pipelines[ dispatchInfo.m_pipeline - 1 ];

    auto& descriptorPool = fr.m_descriptorSets[ currentPipeline.descriptorSetPoolId() ];
    const std::array uniformInfo = {
        fr.m_uniformBuffer.copy( dispatchInfo.m_uniform.ptr, dispatchInfo.m_uniform.size )
    };
    const VkDescriptorSet descriptorSet = descriptorPool.next();
    assert( descriptorSet != VK_NULL_HANDLE );

    std::array<VkDescriptorImageInfo, 2> imageInfo{
        fr.m_renderTarget.imageInfo(),
        fr.m_renderTargetTmp.imageInfo(),
    };
    descriptorPool.update( descriptorSet, uniformInfo, imageInfo, {} );

    vkCmdBindDescriptorSets( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline.layout(), 0, 1, &descriptorSet, 0, nullptr );
    vkCmdBindPipeline( fr.m_cmdColorPass, VK_PIPELINE_BIND_POINT_COMPUTE, currentPipeline );

    const VkExtent2D extent = fr.m_renderTarget.extent();
    vkCmdDispatch( fr.m_cmdColorPass, extent.width / 4, extent.height / 4, 1 );

    std::swap( fr.m_renderTarget, fr.m_renderTargetTmp );
}
