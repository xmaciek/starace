function( set_vs_directory tgt dirname )
    set_target_properties( ${tgt} PROPERTIES
        FOLDER "${dirname}"
    )
endfunction()

function( decl_pak name )
    add_custom_target( ${name} DEPENDS "${CMAKE_BINARY_DIR}/${name}.pak" )
endfunction()

function( make_pak name )
    get_target_property( src ${name} SOURCES )
    add_custom_command( OUTPUT "${CMAKE_BINARY_DIR}/${name}.pak"
        DEPENDS cooker_pak "${src}"
        COMMAND cooker_pak "${CMAKE_BINARY_DIR}/${name}.pak" "\"${src}\""
    )
    add_dependencies( cook ${name} )
    set_vs_directory( ${name} ${name} )
endfunction()
