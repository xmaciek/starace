add_custom_target( model )
set_vs_directory( model "models" )
add_dependencies( cook model )

function( cook_obj file )
    add_custom_target( "model.${file}" DEPENDS "${file}.objc" )
    add_custom_command(
        OUTPUT "${file}.objc"
        DEPENDS cooker_obj
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}.obj"
        COMMAND "${CMAKE_BINARY_DIR}/cooker_obj" "${CMAKE_CURRENT_SOURCE_DIR}/${file}.obj" "${file}.objc"
    )
    add_dependencies( model "model.${file}" )
    set_vs_directory( "model.${file}" "models" )
endfunction()

cook_obj( a2 )
cook_obj( a3 )
cook_obj( a4 )
cook_obj( a5 )
