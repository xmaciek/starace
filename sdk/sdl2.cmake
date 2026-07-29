function( sdl3FromDirectory rootDir includeDir binDir libDir )
    if ( TARGET SDL3::SDL3 )
        return()
    endif()
    if ( NOT EXISTS "${rootDir}/${includeDir}/SDL.h" )
        return()
    endif()

    add_library( sdl3 UNKNOWN IMPORTED GLOBAL )
    add_library( SDL3::SDL3 ALIAS sdl3 )
    set_target_properties( sdl3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${rootDir}/${includeDir}"
        LINK_LIBRARIES "${rootDir}/${binDir}/SDL3.dll"
        IMPORTED_LOCATION "${rootDir}/${libDir}/SDL3.lib"
    )
    configure_file( "${rootDir}/${binDir}/SDL3.dll" "${CMAKE_BINARY_DIR}/SDL3.dll" COPYONLY )

endfunction()

sdl3FromDirectory( "${CMAKE_CURRENT_SOURCE_DIR}/SDL3" "include" "lib/x64" "lib/x64" )
sdl3FromDirectory( "$ENV{VK_SDK_PATH}/Third-Party" "include/SDL3" "bin" "bin" )
sdl3FromDirectory( "$ENV{VK_SDK_PATH}" "include/SDL3" "bin" "lib" )

if ( NOT TARGET SDL3::SDL3 AND UNIX )
    if ( NOT EXISTS "/usr/include/SDL3/SDL.h" )
        return()
    endif()
    add_library( sdl3 UNKNOWN IMPORTED GLOBAL )
    add_library( SDL3::SDL3 ALIAS sdl3 )
    set_target_properties( sdl3 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/usr/include/SDL3"
        IMPORTED_LOCATION "/usr/lib/libSDL3.so"
    )
endif()


if ( NOT TARGET SDL3::SDL3 )
    message( FATAL_ERROR "SDL3 not found" )
endif()
