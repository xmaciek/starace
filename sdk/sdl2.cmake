function( sdl2FromDirectory rootDir includeDir binDir libDir )
    if ( TARGET SDL2::SDL2 )
        return()
    endif()
    if ( NOT EXISTS "${rootDir}/${includeDir}/SDL.h" )
        return()
    endif()

    add_library( sdl2 UNKNOWN IMPORTED GLOBAL )
    add_library( SDL2::SDL2 ALIAS sdl2 )
    set_target_properties( sdl2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${rootDir}/${includeDir}"
        LINK_LIBRARIES "${rootDir}/${binDir}/SDL2.dll"
        IMPORTED_LOCATION "${rootDir}/${libDir}/SDL2.lib"
    )
    configure_file( "${rootDir}/${binDir}/SDL2.dll" "${CMAKE_BINARY_DIR}/SDL2.dll" COPYONLY )

    add_library( sdl2main UNKNOWN IMPORTED GLOBAL )
    add_library( SDL2::main ALIAS sdl2main )
    set_target_properties( sdl2main PROPERTIES
        IMPORTED_LOCATION "${rootDir}/${libDir}/SDL2main.lib"
    )
endfunction()

sdl2FromDirectory( "${CMAKE_CURRENT_SOURCE_DIR}/SDL2" "include" "lib/x64" "lib/x64" )
sdl2FromDirectory( "$ENV{VK_SDK_PATH}/Third-Party" "include/SDL2" "bin" "bin" )
sdl2FromDirectory( "$ENV{VK_SDK_PATH}" "include/SDL2" "bin" "lib" )

if ( NOT TARGET SDL2::SDL2 AND UNIX )
    if ( NOT EXISTS "/usr/include/SDL2/SDL.h" )
        return()
    endif()
    add_library( sdl2 UNKNOWN IMPORTED GLOBAL )
    add_library( SDL2::SDL2 ALIAS sdl2 )
    set_target_properties( sdl2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/usr/include/SDL2"
        IMPORTED_LOCATION "/usr/lib/libSDL2.so"
    )

    add_library( sdl2main UNKNOWN IMPORTED GLOBAL )
    add_library( SDL2::main ALIAS sdl2main )
    set_target_properties( sdl2main PROPERTIES
        LINK_LIBRARIES "/usr/lib/libSDL2main.a"
        IMPORTED_LOCATION "/usr/lib/libSDL2main.a"
    )
endif()


if ( NOT TARGET SDL2::SDL2 )
    message( FATAL_ERROR "SDL2 not found" )
endif()
