
#include "instance.hpp"

#include "wishlist.hpp"

#include <profiler.hpp>

#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <string>

#include <platform/utils.hpp>
#include <platform/linux.hpp>
#include <platform/windows.hpp>

#if PLATFORM_LINUX
static constexpr auto LIB_NAME = "libvulkan.so";
static constexpr auto FLAGS = RTLD_LAZY | RTLD_LOCAL;

#elif PLATFORM_WINDOWS
static constexpr auto LIB_NAME = "vulkan-1";
static constexpr auto FLAGS = 0;
static const auto dlopen = []( auto name, auto ) -> void* { return reinterpret_cast<void*>( LoadLibraryA( name ) ); };
static const auto dlsym = []( void* l, auto name ) -> void* { return GetProcAddress( reinterpret_cast<HMODULE>( l ), name ); };
static const auto dlclose = []( void* l ) { FreeLibrary( reinterpret_cast<HMODULE>( l ) ); };

#else
#error "platform not supported"
#endif

#define DECL_FN( name ) static PFN_##name name = nullptr;
DECL_FN( vkGetInstanceProcAddr );
DECL_FN( vkCreateInstance );
DECL_FN( vkDestroyInstance );
DECL_FN( vkEnumerateInstanceLayerProperties );
DECL_FN( vkEnumerateInstanceExtensionProperties );

static std::pmr::vector<const char*> enabledLayers()
{
    assert( vkEnumerateInstanceLayerProperties );
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


static void enabledExtensions( std::pmr::vector<const char*>& extensionList )
{
    assert( vkEnumerateInstanceExtensionProperties );
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
    std::pmr::vector<VkExtensionProperties> tmp( count );
    vkEnumerateInstanceExtensionProperties( nullptr, &count, tmp.data() );

    [[maybe_unused]]
    Wishlist<VkExtensionProperties> wishlist{ &tmp, &extensionList };
#if ENABLE_VULKAN_VALIDATION
    wishlist( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif
}

Instance::~Instance() noexcept
{
    if ( m_instance && vkDestroyInstance ) vkDestroyInstance( m_instance, nullptr );
    if ( m_dll ) { dlclose( m_dll ); }
}

Instance::Instance( const Renderer::CreateInfo& ci, std::pmr::vector<const char*> extensions ) noexcept
{
    ZoneScoped;
    assert( !ci.gameName.empty() );
    const char* envName = std::getenv( "STARACE_VULKAN_LIBRARY" );
    const char* libName = envName ? envName : LIB_NAME;
    m_dll = dlopen( libName, FLAGS );
    if ( !m_dll ) {
        std::string msg = "Failed to open Vulkan library: ";
        msg.append( libName );
        platform::showFatalError( "Fatal Error", msg );
    }
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>( dlsym( m_dll, "vkGetInstanceProcAddr" ) );

    if ( !vkGetInstanceProcAddr ) {
        platform::showFatalError( "Fatal Error", "Failed to find vkGetInstanceProcAddr in vulkan library" );
    }

    auto makeErrorMessage = [libName]( auto name )
    {
        std::string ret{ "Failed to find " };
        ret.append( name );
        ret.append( " in " );
        ret.append( libName );
        ret.append( " library" );
        return ret;
    };
#define GET_PROC( name ) \
    if ( name = reinterpret_cast<PFN_##name>( procAddr( #name ) ); !name ) \
        platform::showFatalError( "Fatal Error", makeErrorMessage( #name ) )
#define GET_PROC_OPTIONAL( name ) name = reinterpret_cast<PFN_##name>( procAddr( #name ) );

    GET_PROC( vkCreateInstance );
    GET_PROC( vkEnumerateInstanceLayerProperties );
    GET_PROC( vkEnumerateInstanceExtensionProperties );

    const VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = ci.gameName.data(),
        .applicationVersion = VK_MAKE_VERSION( ci.versionMajor, ci.versionMinor, ci.versionPatch ),
        .pEngineName = "starace",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = VK_API_VERSION_1_4,
    };

    m_layers = enabledLayers();
    enabledExtensions( extensions );

    const VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>( m_layers.size() ),
        .ppEnabledLayerNames = m_layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>( extensions.size() ),
        .ppEnabledExtensionNames = extensions.data(),
    };

    [[maybe_unused]]
    const VkResult instanceOK = vkCreateInstance( &instanceCreateInfo, nullptr, &m_instance );
    assert( instanceOK == VK_SUCCESS );

    GET_PROC( vkDestroyInstance );
#define DECL_FUNCTION( name ) GET_PROC( name )
#define DECL_FUNCTION_OPTIONAL( name ) GET_PROC_OPTIONAL( name )
#include "vk.def"
}

Instance::operator VkInstance () const noexcept
{
    return m_instance;
}

std::pmr::vector<const char*> Instance::layers() const noexcept
{
    return m_layers;
}

PFN_vkVoidFunction Instance::procAddr( const char* name ) const noexcept
{
    assert( vkGetInstanceProcAddr );
    return vkGetInstanceProcAddr( m_instance, name );
}

Instance::Instance( Instance&& rhs ) noexcept
{
    std::swap( m_instance, rhs.m_instance );
    std::swap( m_dll, rhs.m_dll );
    std::swap( m_layers, rhs.m_layers );
}

Instance& Instance::operator = ( Instance&& rhs ) noexcept
{
    std::swap( m_instance, rhs.m_instance );
    std::swap( m_dll, rhs.m_dll );
    std::swap( m_layers, rhs.m_layers );
    return *this;
}


