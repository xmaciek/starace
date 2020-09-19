
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


set( CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=.;
    -checks=*,-modernize-use-trailing-return-type,-readability-magic-numbers,-cppcoreguidelines-avoid-magic-numbers,-modernize-use-auto,-hicpp-use-auto,-cppcoreguidelines-pro-bounds-array-to-pointer-decay,-hicpp-no-array-decay,-llvm-header-guard;
)

set( STARACE_BINNAME "starace" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( CMAKE_CXX_STANDARD 11 )
add_executable( ${STARACE_BINNAME} ${STARACE_SOURCES} )
target_link_libraries( ${STARACE_BINNAME} SDL SDL_ttf SDL_mixer GL GLU )
target_compile_options( ${STARACE_BINNAME} PRIVATE -Werror -Wall -Wextra )
