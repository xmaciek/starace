#if defined( _WIN64 )

#define PLATFORM_WINDOWS 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <Shlobj.h>

#else

#define PLATFORM_WINDOWS 0

#endif
