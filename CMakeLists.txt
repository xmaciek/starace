cmake_minimum_required( VERSION 3.20.0 )
project( StarAce )

set( STARACE_MAJOR_VERSION 1 )
set( STARACE_MINOR_VERSION 0 )
set( STARACE_PATCH_VERSION 0 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

include( glm.cmake )

add_executable( cooker_obj )

function( set_cppver tgt cppver )
    set_target_properties( ${tgt} PROPERTIES
        CXX_STANDARD ${cppver}
        CXX_STANDARD_REQUIRED ${cppver}
    )
endfunction()

function( set_error_checks tgt )
    if ( NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
        message( NOTICE "${tgt} extra options " ${ARGN} )
        target_compile_options( ${tgt} PRIVATE -Werror -Wall -Wextra ${ARGN} )
    endif()
endfunction()

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
