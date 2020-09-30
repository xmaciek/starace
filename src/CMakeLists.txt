find_package( Threads REQUIRED )
find_package( Freetype 2 REQUIRED )
find_package( SDL2 REQUIRED )

find_package( glm REQUIRED )
if ( NOT TARGET glm::glm )
    add_library( glm::glm INTERFACE IMPORTED )
    set_target_properties( glm::glm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS}
    )
endif()

set( CMAKE_CXX_CLANG_TIDY clang-tidy )

add_subdirectory( audio )
add_subdirectory( renderer )

set( STARACE_HEADERS
    bullet.hpp
    button.hpp
    circle.hpp
    colors.hpp
    enemy.hpp
    font.hpp
    fps_meter.hpp
    game.hpp
    jet.hpp
    label.hpp
    map.hpp
    model.hpp
    model_proto.hpp
    reactor.hpp
    render_context.hpp
    saobject.hpp
    shield.hpp
    texture.hpp
    thruster.hpp
    update_context.hpp
    utils.hpp
)

set( STARACE_SOURCES
    bullet.cpp
    button.cpp
    circle.cpp
    enemy.cpp
    font.cpp
    fps_meter.cpp
    game.cpp
    game_input.cpp
    game_rendering.cpp
    jet.cpp
    label.cpp
    main.cpp
    map.cpp
    model.cpp
    reactor.cpp
    saobject.cpp
    shield.cpp
    texture.cpp
    thruster.cpp
    utils.cpp
)



set( STARACE_BINNAME "starace" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

add_executable( ${STARACE_BINNAME} ${STARACE_SOURCES} ${STARACE_HEADERS} )
target_compile_options( ${STARACE_BINNAME} PRIVATE -Werror -Wall -Wextra )
target_compile_definitions( ${STARACE_BINNAME} PRIVATE
    GLM_FORCE_RADIANS=1
)

set_target_properties( ${STARACE_BINNAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
)

target_link_libraries( ${STARACE_BINNAME}
    audio
    renderer_gl
    Freetype::Freetype
    SDL2::SDL2
    Threads::Threads
    glm::glm
)

