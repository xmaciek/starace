cmake_minimum_required( VERSION 3.5.1 )
project( StarAce )

set( STARACE_MAJOR_VERSION 1 )
set( STARACE_MINOR_VERSION 1 )
set( STARACE_PATCH_VERSION 0 )

find_package( SDL REQUIRED)
find_package( SDL_mixer REQUIRED)
find_package( SDL_ttf REQUIRED)
find_package( OpenGL REQUIRED )

add_compile_options( -Wall -O0 -g -std=c++11 )

add_subdirectory( misc )
add_subdirectory( models )
add_subdirectory( sounds )
add_subdirectory( src )
add_subdirectory( textures )

configure_file( README README COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( config.cfg config.cfg COPYONLY )
configure_file( jets.cfg jets.cfg COPYONLY )
configure_file( maps.cfg maps.cfg COPYONLY )
configure_file( vertexShader.vert vertexShader.vert COPYONLY )
configure_file( fragmentShader.frag fragmentShader.frag COPYONLY )
