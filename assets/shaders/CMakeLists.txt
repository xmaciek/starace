function( compileShader file )
    #set( OPTIMIZE_LEVEL "-O0" "-g" ) # debug
    set( OPTIMIZE_LEVEL "-O" ) # optimized

    set( file_out "${CMAKE_CURRENT_BINARY_DIR}/${file}.spv" )
    add_custom_target( "shader.${file}" DEPENDS "${file_out}" )
    add_custom_command(
        OUTPUT "${file_out}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
        COMMAND Vulkan::glslc ${OPTIMIZE_LEVEL} "${CMAKE_CURRENT_SOURCE_DIR}/${file}" -o "${file_out}"
    )
    set_vs_directory( "shader.${file}" "shaders" )
    pak_file_cooked( data "${file_out}" "shader.${file}" )
endfunction()

compileShader( albedo.frag )
compileShader( albedo.vert )
compileShader( background.frag )
compileShader( background.vert )
compileShader( gamma.comp )
compileShader( glow.frag )
compileShader( glow.vert )
compileShader( line3_strip_color.frag )
compileShader( line3_strip_color.vert )
compileShader( lines_color1.frag )
compileShader( lines_color1.vert )
compileShader( particles_blob.frag )
compileShader( particles_blob.vert )
compileShader( scanline.comp )
compileShader( space_dust.vert )
compileShader( sprite_sequence.frag )
compileShader( sprite_sequence.vert )
compileShader( sprite_sequence_colors.frag )
compileShader( sprite_sequence_colors.vert )
compileShader( thruster.frag )
compileShader( thruster.vert )
compileShader( trianglefan_color.frag )
compileShader( trianglefan_color.vert )
compileShader( trianglefan_texture.frag )
compileShader( trianglefan_texture.vert )
compileShader( ui_rings.frag )
compileShader( ui_rings.vert )
compileShader( beam_blob.frag )
compileShader( beam_blob.vert )
