add_custom_target( shader )

function( compileShader file )
    add_custom_target( "shader.${file}" DEPENDS "${file}.spv" )
    add_custom_command(
        OUTPUT "${file}.spv"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
        COMMAND glslc "${CMAKE_CURRENT_SOURCE_DIR}/${file}" -o "${file}.spv"
    )
    add_dependencies( shader "shader.${file}" )
endfunction()

compileShader( gui_texture_color.vert )
compileShader( gui_texture_color.frag )
