find_package( SDL2 REQUIRED )
find_package( glm REQUIRED )

if ( NOT TARGET glm::glm )
    add_library( glm::glm INTERFACE IMPORTED )
    set_target_properties( glm::glm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS}
    )
endif()


add_library( renderer INTERFACE )
target_sources( renderer
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/renderer.hpp
)

target_compile_options( renderer INTERFACE -Werror -Wall -Wextra )

set_target_properties( renderer PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_compile_definitions( renderer INTERFACE
    GLM_FORCE_RADIANS=1
)

target_link_libraries( renderer
    INTERFACE
    glm::glm
    SDL2::SDL2
)

add_subdirectory( opengl )
add_subdirectory( vulkan )
