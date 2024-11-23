#pragma once

#if ENABLE_TRACY_PROFILER

#include <tracy/Tracy.hpp>

#else

#define FrameMark {}
#define ZoneScoped {}
#define ZoneScopedN( ... ) {}
#define TracyAlloc( ptr, size ) {}
#define TracyFree( ptr ) {}

#endif
