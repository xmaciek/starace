
find_package( glm REQUIRED )
if ( TARGET glm::glm )
    return()
endif()

add_library( glm::glm INTERFACE IMPORTED )

target_compile_definitions( glm::glm INTERFACE
    GLM_FORCE_RADIANS=1
)

set_target_properties( glm::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS}
)
