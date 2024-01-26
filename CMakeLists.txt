cmake_minimum_required( VERSION 3.24.0 )
project( StarAce )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
add_custom_target( cook ALL )

include( compiler.cmake )
include( functions.cmake )

find_package( Vulkan REQUIRED )


decl_pak( fonts )
decl_pak( shaders )
decl_pak( models )
decl_pak( sounds )
decl_pak( textures )
decl_pak( uis )
decl_pak( misc )
add_subdirectory( assets )
add_subdirectory( sdk )
add_subdirectory( src )
add_subdirectory( tests )
make_pak( fonts )
make_pak( shaders )
make_pak( models )
make_pak( sounds )
make_pak( textures )
make_pak( uis )
make_pak( misc )


configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( SDL2_gamecontrollerdb.txt SDL2_gamecontrollerdb.txt COPYONLY )
