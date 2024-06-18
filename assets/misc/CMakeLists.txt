function( cook_atlas )
    set( oneValueArgs SRC DST )
    cmake_parse_arguments( COOK_ATLAS "" "${oneValueArgs}" "" ${ARGN} )
    if ( NOT DEFINED COOK_ATLAS_SRC )
        message( FATAL_ERROR "SRC not provided" )
    endif()
    if ( NOT DEFINED COOK_ATLAS_DST )
        message( FATAL_ERROR "DST file not provided" )
    endif()

    add_custom_target( atlas.${COOK_ATLAS_DST}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
    )

    set_vs_directory( atlas.${COOK_ATLAS_DST} "misc" )
    pak_file_cooked( data "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}" atlas.${COOK_ATLAS_DST} )

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
        DEPENDS cooker_atlas "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_ATLAS_SRC}"
        COMMAND cooker_atlas
            -i "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_ATLAS_SRC}"
            -o "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
    )
endfunction()

cook_atlas( SRC "ui_atlas.txt" DST "ui_atlas.fnta" )
