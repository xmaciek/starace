#include "vk.hpp"

#include <cassert>
#include <cstdlib>

#if defined( __linux__ )
#include <dlfcn.h>
static constexpr auto LIB_NAME = "libvulkan.so";
static constexpr auto FLAGS = RTLD_LAZY | RTLD_LOCAL;

#elif defined( _WIN64 )
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

static constexpr auto LIB_NAME = "vulkan-1";
static constexpr auto FLAGS = 0;
static auto dlopen = []( auto name, auto ) { return LoadLibraryA( name ); };
static auto& dlsym = GetProcAddress;
static auto& dlclose = FreeLibrary;

#else
#error "platform not supported"
#endif


namespace vk {

static void* dllHandle = nullptr;

bool dllLoad()
{
    const char* envName = std::getenv( "STARACE_VULKAN_LIBRARY" );
    dllHandle = dlopen( envName ? envName : LIB_NAME, FLAGS );
    if ( !dllHandle ) {
        return false;
    }

#define DECL_FUNCTION( name ) \
    auto tmp_##name = reinterpret_cast<PFN_##name>( dlsym( dllHandle, #name ) ); \
    if ( !tmp_##name ) { assert( !"failed to resolve function" ); return false; }
#include "vk.def"

#define DECL_FUNCTION( name ) name = tmp_##name;
#include "vk.def"
    return true;
}

void dllUnload()
{
    void* dll = dllHandle;
    dllHandle = nullptr;
    if ( !dll ) { return; }
#define DECL_FUNCTION( name ) name = nullptr;
#include "vk.def"
    dlclose( dll );
}

}
