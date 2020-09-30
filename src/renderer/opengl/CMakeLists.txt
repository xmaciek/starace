set( OpenGL_GL_PREFERENCE "GLVND" )
find_package( OpenGL REQUIRED )

add_library( renderer_gl STATIC )
target_sources( renderer_gl
    PRIVATE
    renderer_gl.cpp
)

target_compile_options( renderer_gl PRIVATE -Werror -Wall -Wextra )

set_target_properties( renderer_gl PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
)

target_link_libraries( renderer_gl
    renderer
    OpenGL::GL
    OpenGL::GLU
)
