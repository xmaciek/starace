if ( ${ENABLE_TRACY_PROFILER} )
    add_library( profiler SHARED )
    target_compile_definitions( profiler INTERFACE ENABLE_TRACY_PROFILER=1 )
    target_link_libraries( profiler Tracy::TracyClient )
else()
    add_library( profiler INTERFACE )
    target_compile_definitions( profiler INTERFACE ENABLE_TRACY_PROFILER=0 )
endif()

target_sources( profiler
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/profiler.hpp"
)

set_target_properties( profiler PROPERTIES
    INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_vs_directory( profiler "libs" )
