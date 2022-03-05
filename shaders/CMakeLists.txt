add_custom_target( shader )
add_dependencies( cook shader )

function( compileShader file )
    add_custom_target( "shader.${file}" DEPENDS "${file}.spv" )
    add_custom_command(
        OUTPUT "${file}.spv"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
        COMMAND Vulkan::glslc "${CMAKE_CURRENT_SOURCE_DIR}/${file}" -o "${file}.spv"
    )
    add_dependencies( shader "shader.${file}" )
endfunction()

compileShader( gui_texture_color.vert )
compileShader( gui_texture_color.frag )

compileShader( albedo.vert )
compileShader( albedo.frag )

compileShader( line3_strip_color.vert )
compileShader( line3_strip_color.frag )

compileShader( trianglefan_color.vert )
compileShader( trianglefan_color.frag )

compileShader( trianglefan_texture.vert )
compileShader( trianglefan_texture.frag )

compileShader( lines_color1.vert )
compileShader( lines_color1.frag )

compileShader( sprite_sequence.vert )
compileShader( sprite_sequence.frag )

compileShader( progressbar.vert )
compileShader( progressbar.frag )

compileShader( glow.vert )
compileShader( glow.frag )

compileShader( background.vert )
compileShader( background.frag )

compileShader( sprite3d.vert )
compileShader( sprite3d.frag )

compileShader( thruster.vert )
compileShader( thruster.frag )
