option( ENABLE_TRACY_PROFILER "Whether to enable tracy profiler" FALSE )

if ( ${ENABLE_TRACY_PROFILER} )
    set( TRACY_CALLSTACK ON CACHE BOOL "" )
    set( TRACY_ENABLE ON CACHE BOOL "" )
    add_subdirectory( tracy )
    set_vs_directory( TracyClient "sdk" )
endif()
