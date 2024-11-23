option( ENABLE_TRACY_PROFILER "Whether to enable tracy profiler" FALSE )

if ( ${ENABLE_TRACY_PROFILER} )
    set( TRACY_CALLSTACK ON )
    set( TRACY_ENABLE ON )
    add_subdirectory( tracy )
    set_vs_directory( TracyClient "sdk" )
endif()
