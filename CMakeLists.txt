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

function( pack )
    set( oneValueArgs ARCHIVE )
    set( multiValueArgs FILES )
    cmake_parse_arguments( PACK "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    if ( NOT DEFINED PACK_ARCHIVE )
        message( FATAL_ERROR "ARCHIVE file not provided" )
    endif()

    if ( NOT DEFINED PACK_FILES )
        message( FATAL_ERROR "FILES list not provided" )
    endif()

    set( PACK_TARGET "pack.${PACK_ARCHIVE}" )
    set( PACK_TARGET_FILE "${CMAKE_BINARY_DIR}/${PACK_ARCHIVE}.pak" )
    add_custom_target( "${PACK_TARGET}" DEPENDS "${PACK_TARGET_FILE}" )
    set_vs_directory( ${PACK_TARGET} "pack" )
    add_custom_command(
        OUTPUT "${PACK_TARGET_FILE}"
        DEPENDS cooker_pak "${PACK_FILES}"
        COMMAND cooker_pak "${PACK_TARGET_FILE}" "\"${PACK_FILES}\""
    )
    add_dependencies( cook pack.${PACK_ARCHIVE} )
endfunction()

add_custom_target( cook ALL )

find_package( Vulkan REQUIRED )
add_subdirectory( assets )
add_subdirectory( sdk )
add_subdirectory( src )
add_subdirectory( tests )

configure_file( README.md README.md COPYONLY )
configure_file( LICENSE LICENSE COPYONLY )
configure_file( SDL2_gamecontrollerdb.txt SDL2_gamecontrollerdb.txt COPYONLY )

