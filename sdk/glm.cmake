function( glmFromDirectory directory )
    if ( TARGET glm::glm )
        return()
    endif()
    if ( NOT EXISTS "${directory}/glm/glm.hpp" )
        return()
    endif()
    add_library( xglm INTERFACE IMPORTED GLOBAL )
    add_library( glm::glm ALIAS xglm )
    set_target_properties( xglm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${directory}"
    )
    target_compile_definitions( xglm INTERFACE
        GLM_FORCE_RADIANS=1
        GLM_FORCE_INTRINSICS=1
        GLM_FORCE_DEFAULT_ALIGNED_GENTYPES=1
    )
endfunction()

glmFromDirectory( "${CMAKE_CURRENT_SOURCE_DIR}" )
glmFromDirectory( "$ENV{VK_SDK_PATH}/Third-Party/Include" )
glmFromDirectory( "$ENV{VK_SDK_PATH}/Include" )

if ( NOT TARGET glm::glm AND UNIX )
    find_package( glm REQUIRED )
    glmFromDirectory( "${GLM_INCLUDE_DIRS}" )
endif()

if ( NOT TARGET glm::glm )
    message( FATAL_ERROR "GLM not found" )
endif()
