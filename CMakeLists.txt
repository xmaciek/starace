cmake_minimum_required( VERSION 3.24.0 )
project( StarAce )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

add_custom_target( cook ALL )

include( compiler.cmake )
include( functions.cmake )

find_package( Vulkan REQUIRED )


decl_pak( data )
set( DEFAULT_PACK data )

add_subdirectory( assets )
add_subdirectory( sdk )
add_subdirectory( src )
add_subdirectory( tests )
make_pak( data )


configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( SDL2_gamecontrollerdb.txt SDL2_gamecontrollerdb.txt COPYONLY )
