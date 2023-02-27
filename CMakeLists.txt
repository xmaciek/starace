cmake_minimum_required( VERSION 3.24.0 )
project( StarAce )

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

function( pack_tar )
    set( oneValueArgs ARCHIVE )
    set( multiValueArgs FILES )
    cmake_parse_arguments( PACK_TAR "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    if ( NOT DEFINED PACK_TAR_ARCHIVE )
        message( FATAL_ERROR "ARCHIVE file not provided" )
    endif()

    if ( NOT DEFINED PACK_TAR_FILES )
        message( FATAL_ERROR "FILES list not provided" )
    endif()

    add_custom_target( pack.${PACK_TAR_ARCHIVE} DEPENDS "${CMAKE_BINARY_DIR}/${PACK_TAR_ARCHIVE}" )
    set_vs_directory( pack.${PACK_TAR_ARCHIVE} "pack" )
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/${PACK_TAR_ARCHIVE}"
        DEPENDS cooker_tar "${PACK_TAR_FILES}"
        COMMAND cooker_tar "${CMAKE_BINARY_DIR}/${PACK_TAR_ARCHIVE}" "\"${PACK_TAR_FILES}\""
    )
    add_dependencies( cook pack.${PACK_TAR_ARCHIVE} )
endfunction()

add_custom_target( cook ALL )

add_executable( cooker_obj )
set_vs_directory( cooker_obj cooker )

add_executable( cooker_dds )
set_vs_directory( cooker_dds cooker )

add_executable( cooker_tar )
set_vs_directory( cooker_tar cooker )


find_package( Vulkan REQUIRED )
add_subdirectory( sdk )
add_subdirectory( models )
add_subdirectory( shaders )
add_subdirectory( sounds )
add_subdirectory( src )
add_subdirectory( textures )
add_subdirectory( tests )
add_subdirectory( ui )

configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( SDL2_gamecontrollerdb.txt SDL2_gamecontrollerdb.txt COPYONLY )

list( APPEND misc_list "${CMAKE_CURRENT_SOURCE_DIR}/misc/jets.cfg" )
list( APPEND misc_list "${CMAKE_CURRENT_SOURCE_DIR}/misc/maps.cfg" )
list( APPEND misc_list "${CMAKE_CURRENT_SOURCE_DIR}/misc/weapons.cfg" )
list( APPEND misc_list "${CMAKE_CURRENT_SOURCE_DIR}/lang/en.txt" )
list( APPEND misc_list "${CMAKE_CURRENT_SOURCE_DIR}/misc/DejaVuSans-Bold.ttf" )
pack_tar( ARCHIVE "misc.tar" FILES "${misc_list}" )
