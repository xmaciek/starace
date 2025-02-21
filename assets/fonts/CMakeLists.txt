function( cook_font )
    set( oneValueArgs TARGET SRC SIZE )
    cmake_parse_arguments( COOK_FONT "" "${oneValueArgs}" "RANGES" ${ARGN} )
    if ( NOT DEFINED COOK_FONT_TARGET )
        message( FATAL_ERROR "TARGET not provided" )
    endif()
    if ( NOT DEFINED COOK_FONT_SRC )
        message( FATAL_ERROR "SRC file not provided" )
    endif()
    if ( NOT DEFINED COOK_FONT_SIZE )
        message( FATAL_ERROR "SIZE not specified" )
    endif()
    if ( NOT DEFINED COOK_FONT_RANGES )
        message( FATAL_ERROR "RANGES not specified" )
    endif()

    set( tgt "${COOK_FONT_TARGET}_${COOK_FONT_SIZE}" )
    set( out.dds "${tgt}.dds" )
    set( out.font "${tgt}.fnta" )
    add_custom_target( font.${tgt}
        DEPENDS "${out.dds}" "${out.font}"
    )
    set_vs_directory( font.${tgt} "assets/fonts" )
    pak_file_cooked( data "${CMAKE_CURRENT_BINARY_DIR}/${out.font}" font.${tgt} )
    pak_file_cooked( data "${CMAKE_CURRENT_BINARY_DIR}/${out.dds}" font.${tgt} )

    add_custom_command(
        OUTPUT "${out.dds}" "${out.font}"
        DEPENDS cooker_font "${COOK_FONT_SRC}"
        COMMAND cooker_font
            --px "${COOK_FONT_SIZE}"
            --src "${COOK_FONT_SRC}"
            --out "${CMAKE_CURRENT_BINARY_DIR}/${out.font}"
            --dds "${CMAKE_CURRENT_BINARY_DIR}/${out.dds}"
            --ranges "\"${COOK_FONT_RANGES}\""
    )
endfunction()

set( LATIN_RANGE "20,7F,A1,180" )

cook_font( SRC "${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans-Bold.ttf" TARGET dejavu SIZE 24 RANGES ${LATIN_RANGE} )
cook_font( SRC "${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans-Bold.ttf" TARGET dejavu SIZE 36 RANGES ${LATIN_RANGE} )
cook_font( SRC "${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans-Bold.ttf" TARGET dejavu SIZE 64 RANGES ${LATIN_RANGE} )
