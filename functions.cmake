function( set_vs_directory tgt dirname )
    set_target_properties( ${tgt} PROPERTIES
        FOLDER "${dirname}"
    )
endfunction()

function( decl_pak name )
    add_custom_target( ${name} DEPENDS "${CMAKE_BINARY_DIR}/${name}.pak" )
endfunction()

function( make_pak name )
    if ( NOT TARGET ${name} )
        message( FATAL_ERROR "archive ${name} not declared, check name typos" )
    endif()
    get_target_property( src ${name} SOURCES )
    add_custom_command( OUTPUT "${CMAKE_BINARY_DIR}/${name}.pak"
        DEPENDS cooker_pak "${src}"
        COMMAND cooker_pak "${CMAKE_BINARY_DIR}/${name}.pak" "\"${src}\""
    )
    add_dependencies( cook ${name} )
    set_vs_directory( ${name} ${name} )
endfunction()

function( pak_file_cooked archive file depends )
    if ( NOT TARGET ${archive} )
        message( FATAL_ERROR "archive ${archive} not declared, check name typos" )
    endif()
    target_sources( ${archive} PRIVATE "${file}" )
    if ( TARGET ${depends} )
        add_dependencies( ${archive} "${depends}" )
    endif()
endfunction()

function( pak_file archive file )
    if ( NOT TARGET ${archive} )
        message( FATAL_ERROR "archive ${archive} not declared, check name typos" )
    endif()
    target_sources( ${archive} PRIVATE "${file}" )
endfunction()

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

function( cook_dds )
    cmake_parse_arguments( COOK_DDS "MIPGEN;CUBEMAP" "DST;FORMAT;PACK" "SRC" ${ARGN} )
    if ( COOK_DDS_MIPGEN )
        set( COOK_DDS_MIPGEN "--mipgen" )
    endif()
    if ( COOK_DDS_CUBEMAP )
        set( COOK_DDS_CUBEMAP "--cubemap" )
    endif()
    if ( COOK_DDS_FORMAT )
        set( COOK_DDS_FORMAT_PARAM "--format" )
    endif()

    if ( NOT COOK_DDS_SRC )
        message( FATAL_ERROR "SRC argument not specified" )
    endif()
    if( NOT COOK_DDS_DST )
        message( FATAL_ERROR "DST argument not specified" )
    endif()
    if ( NOT COOK_DDS_PACK )
        set( COOK_DDS_PACK ${DEFAULT_PACK} )
    endif()

    set( file_out "${CMAKE_CURRENT_BINARY_DIR}/${COOK_DDS_DST}" )
    add_custom_target( "texture.${COOK_DDS_DST}" DEPENDS "${file_out}" )

    set( expaned_src )
    foreach ( src ${COOK_DDS_SRC} )
        list( APPEND expaned_src "${CMAKE_CURRENT_SOURCE_DIR}/${src}" )
    endforeach()
    list( JOIN expaned_src "," srcArg )

    add_custom_command(
        OUTPUT "${file_out}"
        DEPENDS cooker_dds
        DEPENDS "${expaned_src}"
        COMMAND cooker_dds
            --src "${srcArg}"
            --dst "${file_out}"
            ${COOK_DDS_MIPGEN}
            ${COOK_DDS_CUBEMAP}
            ${COOK_DDS_FORMAT_PARAM} ${COOK_DDS_FORMAT}
    )
    set_vs_directory( "texture.${COOK_DDS_DST}" "assets/textures" )
    pak_file_cooked( ${COOK_DDS_PACK} "${file_out}" "texture.${COOK_DDS_DST}" )
endfunction()

function( cook_atlas )
    cmake_parse_arguments( COOK_ATLAS "REMAP" "SRC;DST;TEXTURE;PACK" "" ${ARGN} )
    if ( NOT DEFINED COOK_ATLAS_SRC )
        message( FATAL_ERROR "SRC not provided" )
    elseif ( NOT DEFINED COOK_ATLAS_DST )
        message( FATAL_ERROR "DST file not provided" )
    elseif ( NOT DEFINED COOK_ATLAS_TEXTURE )
        message( FATAL_ERROR "TEXTURE file not provided" )
    endif()
    if ( NOT DEFINED COOK_ATLAS_PACK )
        set( COOK_ATLAS_PACK ${DEFAULT_PACK} )
    endif()

    if ( COOK_ATLAS_REMAP )
        set( COOK_ATLAS_REMAP "--remap" )
    else()
        set( COOK_ATLAS_REMAP "" )
    endif()

    add_custom_target( atlas.${COOK_ATLAS_DST}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
    )

    set_vs_directory( atlas.${COOK_ATLAS_DST} "assets/misc" )
    pak_file_cooked( ${COOK_ATLAS_PACK} "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}" atlas.${COOK_ATLAS_DST} )

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
        DEPENDS cooker_atlas "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_ATLAS_SRC}"
        COMMAND cooker_atlas
            -i "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_ATLAS_SRC}"
            -o "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
            -t "${COOK_ATLAS_TEXTURE}"
            ${COOK_ATLAS_REMAP}
    )
endfunction()

function( cook_font )
    cmake_parse_arguments( COOK_FONT "" "NAME;SRC;SIZE" "RANGES" ${ARGN} )
    if ( NOT DEFINED COOK_FONT_NAME )
        message( FATAL_ERROR "NAME not provided" )
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

    set( tgt "${COOK_FONT_NAME}" )
    set( out.dds "${tgt}.dds" )
    set( out.font "${tgt}.fnta" )
    add_custom_target( font.${tgt}
        DEPENDS "${out.dds}" "${out.font}"
    )
    set_vs_directory( font.${tgt} "assets/fonts" )
    pak_file_cooked( ${DEFAULT_PACK} "${CMAKE_CURRENT_BINARY_DIR}/${out.font}" font.${tgt} )
    pak_file_cooked( ${DEFAULT_PACK} "${CMAKE_CURRENT_BINARY_DIR}/${out.dds}" font.${tgt} )

    add_custom_command(
        OUTPUT "${out.dds}" "${out.font}"
        DEPENDS cooker_font "${COOK_FONT_SRC}"
        COMMAND cooker_font
            --px "${COOK_FONT_SIZE}"
            --src "${COOK_FONT_SRC}"
            --out "${CMAKE_CURRENT_BINARY_DIR}/${out.font}"
            --dds "${CMAKE_CURRENT_BINARY_DIR}/${out.dds}"
            --ranges "\"${COOK_FONT_RANGES}\""
            --name "${COOK_FONT_NAME}"
    )
endfunction()

function( compileShader )
    cmake_parse_arguments( COOK_SHADER "" "FILE;PACK" "" ${ARGN} )
    if ( NOT COOK_SHADER_FILE )
        message( FATAL_ERROR "FILE argument not specified" )
    endif()
    if ( NOT COOK_SHADER_PACK )
        set( COOK_SHADER_PACK ${DEFAULT_PACK} )
    endif()

    #set( OPTIMIZE_LEVEL "-O0" "-g" ) # debug
    set( OPTIMIZE_LEVEL "-O" ) # optimized

    set( file_out "${CMAKE_CURRENT_BINARY_DIR}/${COOK_SHADER_FILE}.spv" )
    add_custom_target( "shader.${COOK_SHADER_FILE}" DEPENDS "${file_out}" )
    add_custom_command(
        OUTPUT "${file_out}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_SHADER_FILE}"
        COMMAND Vulkan::glslc ${OPTIMIZE_LEVEL} "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_SHADER_FILE}" -o "${file_out}"
    )
    set_vs_directory( "shader.${COOK_SHADER_FILE}" "assets/shaders" )
    pak_file_cooked( ${COOK_SHADER_PACK} "${file_out}" "shader.${COOK_SHADER_FILE}" )
endfunction()

function( declare_cooker )
    cmake_parse_arguments( COOKER "" "NAME" "SRC;LINK" ${ARGN} )
    if ( NOT COOKER_NAME )
        message( FATAL_ERROR "cooker name not set" )
    elseif ( NOT COOKER_SRC )
        message( FATAL_ERROR "cooker sources not set" )
    endif()

    add_executable( ${COOKER_NAME} )
    target_sources( ${COOKER_NAME} PRIVATE ${COOKER_SRC} )
    target_link_libraries( ${COOKER_NAME} cxx::flags extra cooker_common )
    if ( COOKER_LINK )
        target_link_libraries( ${COOKER_NAME} ${COOKER_LINK} )
    endif()
    set_vs_directory( ${COOKER_NAME} cooker )
endfunction()
