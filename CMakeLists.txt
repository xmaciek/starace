cmake_minimum_required( VERSION 3.20.0 )
project( StarAce )

set( STARACE_MAJOR_VERSION 1 )
set( STARACE_MINOR_VERSION 0 )
set( STARACE_PATCH_VERSION 0 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

include( compiler.cmake )

function( set_cppver tgt cppver )
    set_target_properties( ${tgt} PROPERTIES
        CXX_STANDARD ${cppver}
        CXX_STANDARD_REQUIRED ${cppver}
    )
endfunction()
function( set_vs_directory tgt dirname )
    set_target_properties( ${tgt} PROPERTIES
        FOLDER "${dirname}"
    )
endfunction()

add_custom_target( cook ALL )

add_executable( cooker_obj )
set_vs_directory( cooker_obj cooker )

add_executable( cooker_dds )
set_vs_directory( cooker_obj cooker )


find_package( Vulkan REQUIRED )
add_subdirectory( sdk )
add_subdirectory( lang )
add_subdirectory( misc )
add_subdirectory( models )
add_subdirectory( shaders )
add_subdirectory( sounds )
add_subdirectory( src )
add_subdirectory( textures )
add_subdirectory( tests )
add_subdirectory( ui )

configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( jets.cfg jets.cfg COPYONLY )
configure_file( maps.cfg maps.cfg COPYONLY )
configure_file( weapons.cfg weapons.cfg COPYONLY )
configure_file( SDL2_gamecontrollerdb.txt SDL2_gamecontrollerdb.txt COPYONLY )
