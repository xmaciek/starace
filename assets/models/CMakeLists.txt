function( cook_obj file )
    set( file_out "${CMAKE_CURRENT_BINARY_DIR}/${file}.objc" )
    add_custom_target( "model.${file}" DEPENDS "${file_out}" )
    add_custom_command(
        OUTPUT "${file_out}"
        DEPENDS cooker_obj
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}.obj"
        COMMAND cooker_obj "${CMAKE_CURRENT_SOURCE_DIR}/${file}.obj" "${file_out}"
    )
    set_vs_directory( "model.${file}" "models" )
    pak_file_cooked( data "${file_out}" "model.${file}" )
endfunction()

cook_obj( a2 )
cook_obj( a3 )
cook_obj( a4 )
cook_obj( a5 )
cook_obj( cannon )
cook_obj( blaster )
cook_obj( rocket )
