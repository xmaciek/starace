add_custom_target( texture )
add_dependencies( cook texture )

function( cook_tga file )
    add_custom_target( "texture.${file}" DEPENDS "${file}" )
    add_custom_command(
        OUTPUT "${file}"
        DEPENDS cooker_tga
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
        COMMAND "${CMAKE_BINARY_DIR}/cooker_tga" "${CMAKE_CURRENT_SOURCE_DIR}/${file}" "${file}"
    )
    add_dependencies( texture "texture.${file}" )
endfunction()


cook_tga( a2.tga            a2.tga )
cook_tga( a3.tga            a3.tga )
cook_tga( a4.tga            a4.tga )
cook_tga( a5.tga            a5.tga )
cook_tga( button1.tga       button1.tga )
cook_tga( cyber_ring1.tga   cyber_ring1.tga )
cook_tga( cyber_ring2.tga   cyber_ring2.tga )
cook_tga( cyber_ring3.tga   cyber_ring3.tga )
cook_tga( horizon2.tga      horizon2.tga )
cook_tga( horizon_bottom.tga    horizon_bottom.tga )
cook_tga( horizon.tga       horizon.tga )
cook_tga( horizon_top.tga   horizon_top.tga )
cook_tga( nebula1.tga       nebula1.tga )
cook_tga( star_field.tga    star_field.tga )
cook_tga( plasma.tga        plasma.tga )
