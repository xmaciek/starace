function( cook_lang )
    cmake_parse_arguments( COOK_LANG "" "SRC;DST;ID" "" ${ARGN} )

    if ( NOT COOK_LANG_SRC )
        message( FATAL_ERROR "SRC argument not specified" )
    elseif( NOT COOK_LANG_DST )
        message( FATAL_ERROR "DST argument not specified" )
    elseif( NOT COOK_LANG_ID )
        message( FATAL_ERROR "ID argument not specified" )
    endif()
    set( file_out "${CMAKE_CURRENT_BINARY_DIR}/${COOK_LANG_DST}" )
    add_custom_target( "lang.${COOK_LANG_DST}" DEPENDS "${file_out}" )

    add_custom_command(
        OUTPUT "${file_out}"
        DEPENDS cooker_lang
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_LANG_SRC}"
        COMMAND cooker_lang
            --src "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_LANG_SRC}"
            --dst "${file_out}"
            --id "${COOK_LANG_ID}"
    )
    set_vs_directory( "lang.${COOK_LANG_DST}" "assets/lang" )
    pak_file_cooked( ${DEFAULT_PACK} "${file_out}" "lang.${COOK_LANG_DST}" )
endfunction()


cook_lang( SRC en.csv DST en.lang ID en )
cook_lang( SRC pl.csv DST pl.lang ID pl )
