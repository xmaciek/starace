add_custom_target( texture )
add_dependencies( cook texture )

function( cook_dds file dstFile )
    add_custom_target( "texture.${dstFile}" DEPENDS "${dstFile}" )
    add_custom_command(
        OUTPUT "${dstFile}"
        DEPENDS cooker_dds
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
        COMMAND cooker_dds "${CMAKE_CURRENT_SOURCE_DIR}/${file}" "${dstFile}" ${ARGV2}
    )
    add_dependencies( texture "texture.${dstFile}" )
endfunction()


cook_dds( a2.tga            a2.dds  mipgen )
cook_dds( a3.tga            a3.dds  mipgen )
cook_dds( a4.tga            a4.dds  mipgen )
cook_dds( a5.tga            a5.dds  mipgen )
cook_dds( cyber_ring1.tga   cyber_ring1.dds )
cook_dds( cyber_ring2.tga   cyber_ring2.dds )
cook_dds( cyber_ring3.tga   cyber_ring3.dds )
cook_dds( horizon2.tga      horizon2.dds mipgen )
cook_dds( horizon_bottom.tga    horizon_bottom.dds  mipgen )
cook_dds( horizon.tga       horizon.dds     mipgen )
cook_dds( horizon_top.tga   horizon_top.dds mipgen )
cook_dds( nebula1.tga       nebula1.dds     mipgen )
cook_dds( star_field.tga    star_field.dds  mipgen )
cook_dds( plasma.tga        plasma.dds      mipgen )
cook_dds( atlas_ui.tga      atlas_ui.dds )
