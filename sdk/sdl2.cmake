function( sdl2FromDirectory directory libdir )
    if ( TARGET SDL2::SDL2 )
        return()
    endif()
    if ( NOT EXISTS "${directory}/include/SDL2.h" )
        return()
    endif()

    add_library( sdl2 UNKNOWN IMPORTED GLOBAL )
    add_library( SDL2::SDL2 ALIAS sdl2 )
    set_target_properties( sdl2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${directory}/include"
        LINK_LIBRARIES "${directory}/${libdir}/SDL2.dll"
        IMPORTED_LOCATION "${directory}/${libdir}/SDL2.lib"
    )

    add_library( sdl2main UNKNOWN IMPORTED GLOBAL )
    add_library( SDL2::main ALIAS sdl2main )
    set_target_properties( sdl2main PROPERTIES
        IMPORTED_LOCATION "${directory}/${libdir}/SDL2main.lib"
    )
endfunction()

sdl2FromDirectory( "${CMAKE_CURRENT_SOURCE_DIR}/SDL2" "lib/x64" )
sdl2FromDirectory( "$ENV{VK_SDK_PATH}" "bin" )

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
    )
endif()


if ( NOT TARGET SDL2::SDL2 )
    message( FATAL_ERROR "SDL2 not found" )
endif()
