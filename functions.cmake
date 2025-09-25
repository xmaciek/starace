function( set_vs_directory tgt dirname )
    set_target_properties( ${tgt} PROPERTIES
        FOLDER "${dirname}"
    )
endfunction()

function( decl_pak name )
    add_custom_target( ${name} DEPENDS "${CMAKE_BINARY_DIR}/${name}.pak" )
endfunction()

function( make_pak name )
    if ( NOT TARGET ${name} )
        message( FATAL_ERROR "archive ${name} not declared, check name typos" )
    endif()
    get_target_property( src ${name} SOURCES )
    add_custom_command( OUTPUT "${CMAKE_BINARY_DIR}/${name}.pak"
        DEPENDS cooker_pak "${src}"
        COMMAND cooker_pak "${CMAKE_BINARY_DIR}/${name}.pak" "\"${src}\""
    )
    add_dependencies( cook ${name} )
    set_vs_directory( ${name} ${name} )
endfunction()

function( pak_file_cooked archive file depends )
    if ( NOT TARGET ${archive} )
        message( FATAL_ERROR "archive ${archive} not declared, check name typos" )
    endif()
    target_sources( ${archive} PRIVATE "${file}" )
    if ( TARGET ${depends} )
        add_dependencies( ${archive} "${depends}" )
    endif()
endfunction()

function( pak_file archive file )
    if ( NOT TARGET ${archive} )
        message( FATAL_ERROR "archive ${archive} not declared, check name typos" )
    endif()
    target_sources( ${archive} PRIVATE "${file}" )
endfunction()

function( compileShader )
    cmake_parse_arguments( COOK_SHADER "" "FILE;PACK" "" ${ARGN} )
    if ( NOT COOK_SHADER_FILE )
        message( FATAL_ERROR "FILE argument not specified" )
    endif()
    if ( NOT COOK_SHADER_PACK )
        set( COOK_SHADER_PACK ${DEFAULT_PACK} )
    endif()

    #set( OPTIMIZE_LEVEL "-O0" "-g" ) # debug
    set( OPTIMIZE_LEVEL "-O" ) # optimized

    set( file_out "${CMAKE_CURRENT_BINARY_DIR}/${COOK_SHADER_FILE}.spv" )
    add_custom_target( "shader.${COOK_SHADER_FILE}" DEPENDS "${file_out}" )
    add_custom_command(
        OUTPUT "${file_out}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_SHADER_FILE}"
        COMMAND Vulkan::glslc ${OPTIMIZE_LEVEL} "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_SHADER_FILE}" -o "${file_out}"
    )
    set_vs_directory( "shader.${COOK_SHADER_FILE}" "assets/shaders" )
    pak_file_cooked( ${COOK_SHADER_PACK} "${file_out}" "shader.${COOK_SHADER_FILE}" )
endfunction()
