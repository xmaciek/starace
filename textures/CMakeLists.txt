add_custom_target( texture )
set_vs_directory( texture "textures" )
add_dependencies( cook texture )

function( cook_dds )
    cmake_parse_arguments( COOK_DDS "MIPGEN" "DST" "SRC" ${ARGN} )
    if ( COOK_DDS_MIPGEN )
        set( COOK_DDS_MIPGEN "--mipgen" )
    endif()
    if ( NOT COOK_DDS_SRC )
        message( FATAL_ERROR "SRC argument not specified" )
    elseif( NOT COOK_DDS_DST )
        message( FATAL_ERROR "DST argument not specified" )
    endif()
    add_custom_target( "texture.${COOK_DDS_DST}" DEPENDS "${COOK_DDS_DST}" )

    set( expaned_src )
    foreach ( src ${COOK_DDS_SRC} )
        list( APPEND expaned_src "${CMAKE_CURRENT_SOURCE_DIR}/${src}" )
    endforeach()
    list( JOIN expaned_src "," srcArg )

    add_custom_command(
        OUTPUT "${COOK_DDS_DST}"
        DEPENDS cooker_dds
        DEPENDS "${expaned_src}"
        COMMAND cooker_dds
            --src "${srcArg}"
            --dst "${COOK_DDS_DST}"
            ${COOK_DDS_MIPGEN}
    )
    add_dependencies( texture "texture.${COOK_DDS_DST}" )
    set_vs_directory( "texture.${COOK_DDS_DST}" "textures" )

    set( texture_list "${texture_list}" "${CMAKE_CURRENT_BINARY_DIR}/${COOK_DDS_DST}" PARENT_SCOPE )
endfunction()


cook_dds( SRC a2.tga            DST a2.dds  MIPGEN )
cook_dds( SRC a3.tga            DST a3.dds  MIPGEN )
cook_dds( SRC a4.tga            DST a4.dds  MIPGEN )
cook_dds( SRC a5.tga            DST a5.dds  MIPGEN )
cook_dds( SRC atlas_ui.tga      DST atlas_ui.dds )
cook_dds( SRC cyber_ring1.tga;cyber_ring2.tga;cyber_ring3.tga DST cyber_ring.dds MIPGEN )
cook_dds( SRC horizon.tga       DST horizon.dds     MIPGEN )
cook_dds( SRC horizon2.tga      DST horizon2.dds MIPGEN )
cook_dds( SRC horizon_bottom.tga    DST horizon_bottom.dds MIPGEN )
cook_dds( SRC horizon_top.tga   DST horizon_top.dds MIPGEN )
cook_dds( SRC nebula1.tga       DST nebula1.dds     MIPGEN )
cook_dds( SRC plasma.tga        DST plasma.dds      MIPGEN )
cook_dds( SRC star_field.tga    DST star_field.dds  MIPGEN )

pack( ARCHIVE "textures" FILES "${texture_list}" )