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


cook_dds( SRC a2.tga            DST a2.dds  MIPGEN )
cook_dds( SRC a3.tga            DST a3.dds  MIPGEN )
cook_dds( SRC a4.tga            DST a4.dds  MIPGEN )
cook_dds( SRC a5.tga            DST a5.dds  MIPGEN )
cook_dds( SRC a6.tga            DST a6.dds  MIPGEN )
cook_dds( SRC atlas_ui.tga      DST atlas_ui.dds FORMAT BC4 PACK init )
cook_dds( SRC xbox_atlas.tga    DST xbox_atlas.dds FORMAT BC4 )
cook_dds( SRC ps4_atlas.tga     DST ps4_atlas.dds FORMAT BC4 )
cook_dds( SRC horizon.tga       DST horizon.dds     MIPGEN )
cook_dds( SRC horizon2.tga      DST horizon2.dds MIPGEN )
cook_dds( SRC horizon_bottom.tga    DST horizon_bottom.dds MIPGEN )
cook_dds( SRC horizon_top.tga   DST horizon_top.dds MIPGEN )
cook_dds( SRC nebula1.tga       DST nebula1.dds     MIPGEN )
cook_dds( SRC plasma.tga        DST plasma.dds      MIPGEN FORMAT BC4 )
cook_dds( SRC star_field.tga    DST star_field.dds  MIPGEN )
pak_file( data cannon.dds )
pak_file( data blaster.dds )
pak_file( data rocket.dds )
