set( OpenGL_GL_PREFERENCE "GLVND" )
find_package( OpenGL REQUIRED )
find_package( glm REQUIRED )

if ( NOT TARGET glm::glm )
    add_library( glm::glm INTERFACE IMPORTED )
    set_target_properties( glm::glm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS}
    )
endif()


add_library( renderer STATIC )
target_sources( renderer
    PRIVATE
    renderer.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/renderer.hpp
)

target_compile_options( renderer PRIVATE -Werror -Wall -Wextra )

set_target_properties( renderer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_compile_definitions( renderer PRIVATE
    GLM_FORCE_RADIANS=1
)

target_link_libraries( renderer
    glm::glm
    OpenGL::GL
    OpenGL::GLU
)
