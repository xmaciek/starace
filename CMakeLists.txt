cmake_minimum_required( VERSION 3.20.0 )
project( StarAce )

set( STARACE_MAJOR_VERSION 1 )
set( STARACE_MINOR_VERSION 0 )
set( STARACE_PATCH_VERSION 0 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

include( compiler.cmake )

add_custom_target( cook ALL )
add_executable( cooker_obj )
add_executable( cooker_tga )

function( set_cppver tgt cppver )
    set_target_properties( ${tgt} PROPERTIES
        CXX_STANDARD ${cppver}
        CXX_STANDARD_REQUIRED ${cppver}
    )
endfunction()

find_package( Vulkan REQUIRED )
add_subdirectory( sdk )
add_subdirectory( misc )
add_subdirectory( models )
add_subdirectory( shaders )
add_subdirectory( sounds )
add_subdirectory( src )
add_subdirectory( textures )
add_subdirectory( tests )

configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( jets.cfg jets.cfg COPYONLY )
configure_file( maps.cfg maps.cfg COPYONLY )
