find_package( SDL2 REQUIRED )
find_package( glm REQUIRED )

add_library( renderer INTERFACE )
target_sources( renderer
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/texture.hpp
)

target_compile_options( renderer INTERFACE -Werror -Wall -Wextra )

set_target_properties( renderer PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( renderer
    INTERFACE
    glm::glm
    SDL2::SDL2
)

#add_subdirectory( opengl )
add_subdirectory( vulkan )
