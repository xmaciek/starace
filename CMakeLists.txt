cmake_minimum_required( VERSION 3.18.0 )
project( StarAce )

set( STARACE_MAJOR_VERSION 1 )
set( STARACE_MINOR_VERSION 0 )
set( STARACE_PATCH_VERSION 0 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

add_subdirectory( misc )
add_subdirectory( models )
add_subdirectory( shaders )
add_subdirectory( sounds )
add_subdirectory( src )
add_subdirectory( textures )

configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( config.cfg config.cfg COPYONLY )
configure_file( jets.cfg jets.cfg COPYONLY )
configure_file( maps.cfg maps.cfg COPYONLY )
