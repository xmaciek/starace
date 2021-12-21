if ( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/glm/CMakeLists.txt" )
    add_subdirectory( "${CMAKE_CURRENT_SOURCE_DIR}/glm/CMakeLists.txt" )
    target_compile_definitions( glm INTERFACE
        GLM_FORCE_RADIANS=1
    )

else()
    find_package( glm REQUIRED )
    add_library( xglm INTERFACE )
    set_target_properties( xglm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS}
    )
    add_library( glm::glm ALIAS xglm )
    target_compile_definitions( xglm INTERFACE
        GLM_FORCE_RADIANS=1
    )

endif()

if ( NOT TARGET glm::glm )
    message( FATAL_ERROR "GLM not found" )
endif()
