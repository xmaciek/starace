add_custom_target( model )
set_vs_directory( model "models" )
add_dependencies( cook model )

function( cook_obj file )
    add_custom_target( "model.${file}" DEPENDS "${file}.objc" )
    add_custom_command(
        OUTPUT "${file}.objc"
        DEPENDS cooker_obj
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${file}.obj"
        COMMAND cooker_obj "${CMAKE_CURRENT_SOURCE_DIR}/${file}.obj" "${CMAKE_CURRENT_BINARY_DIR}/${file}.objc"
    )
    add_dependencies( model "model.${file}" )
    set_vs_directory( "model.${file}" "models" )
    set( model_list "${model_list}" "${CMAKE_CURRENT_BINARY_DIR}/${file}.objc" PARENT_SCOPE )
endfunction()

cook_obj( a2 )
cook_obj( a3 )
cook_obj( a4 )
cook_obj( a5 )

pack( ARCHIVE "models" FILES "${model_list}" )
