find_package( Threads REQUIRED )

find_package( glm REQUIRED )
if ( NOT TARGET glm::glm )
    add_library( glm::glm INTERFACE IMPORTED )
    set_target_properties( glm::glm PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${GLM_INCLUDE_DIRS}
    )
endif()



set( STARACE_HEADERS
    bullet.hpp
    button.hpp
    circle.hpp
    enemy.hpp
    font.hpp
    game.hpp
    jet.hpp
    map.hpp
    model.hpp
    model_proto.hpp
    reactor.hpp
    render_context.hpp
    render_pipeline.hpp
    renderer.hpp
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
    game.cpp
    game_input.cpp
    game_rendering.cpp
    jet.cpp
    main.cpp
    map.cpp
    model.cpp
    reactor.cpp
    renderer.cpp
    saobject.cpp
    shield.cpp
    texture.cpp
    thruster.cpp
    utils.cpp
)


set( CMAKE_CXX_CLANG_TIDY clang-tidy )

set( STARACE_BINNAME "starace" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( CMAKE_CXX_STANDARD 17 )
add_executable( ${STARACE_BINNAME} ${STARACE_SOURCES} ${STARACE_HEADERS} )
target_compile_options( ${STARACE_BINNAME} PRIVATE -Werror -Wall -Wextra )
target_compile_definitions( ${STARACE_BINNAME} PUBLIC
    GLM_FORCE_RADIANS=1
)

target_link_libraries( ${STARACE_BINNAME}
    Threads::Threads
    glm::glm
    SDL
    SDL_ttf
    SDL_mixer
    GL
    GLU
)

