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
    jet.hpp
    map.hpp
    model.hpp
    quaternion.hpp
    road.hpp
    sa.hpp
    saobject.hpp
    shield.hpp
    tail.hpp
    texture.hpp
    thruster.hpp
)

set( STARACE_SOURCES
    bullet.cpp
    button.cpp
    circle.cpp
    enemy.cpp
    font.cpp
    jet.cpp
    main.cpp
    map.cpp
    model.cpp
    quaternion.cpp
    road.cpp
    road_drawing.cpp
    road_keyboard.cpp
    saobject.cpp
    shield.cpp
    tail.cpp
    texture.cpp
    thruster.cpp
    utils.cpp
)


set( CMAKE_CXX_CLANG_TIDY clang-tidy )

set( STARACE_BINNAME "starace" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( CMAKE_CXX_STANDARD 17 )
add_executable( ${STARACE_BINNAME} ${STARACE_SOURCES} )
target_compile_options( ${STARACE_BINNAME} PRIVATE -Werror -Wall -Wextra )

target_link_libraries( ${STARACE_BINNAME}
    Threads::Threads
    glm::glm
    SDL
    SDL_ttf
    SDL_mixer
    GL
    GLU
)

