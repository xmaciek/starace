find_package( Threads REQUIRED )
find_package( Freetype 2 REQUIRED )
find_package( SDL2 REQUIRED )

set( CMAKE_CXX_CLANG_TIDY clang-tidy )

add_subdirectory( audio )
add_subdirectory( renderer )

set( STARACE_SOURCES
    async_io.cpp
    async_io.hpp
    bullet.cpp
    bullet.hpp
    button.cpp
    button.hpp
    circle.hpp
    colors.hpp
    enemy.cpp
    enemy.hpp
    font.cpp
    font.hpp
    fps_meter.cpp
    fps_meter.hpp
    game.cpp
    game.hpp
    game_input.cpp
    game_rendering.cpp
    jet.cpp
    jet.hpp
    label.cpp
    label.hpp
    main.cpp
    map.cpp
    map.hpp
    model.cpp
    model.hpp
    model_proto.hpp
    obj.cpp
    obj.hpp
    pool.hpp
    reactor.cpp
    reactor.hpp
    render_context.hpp
    saobject.cpp
    saobject.hpp
    shield.cpp
    shield.hpp
    targeting.cpp
    targeting.hpp
    texture.cpp
    texture.hpp
    tga.hpp
    thruster.cpp
    thruster.hpp
    update_context.hpp
    utils.cpp
    utils.hpp
)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( bin_name "starace" )
add_executable( ${bin_name} ${STARACE_SOURCES} )
target_compile_options( ${bin_name} PRIVATE -Werror -Wall -Wextra )
target_compile_definitions( ${bin_name} PRIVATE
    GLM_FORCE_RADIANS=1
)

set_target_properties( ${bin_name} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
)

target_link_libraries( ${bin_name}
    audio
    renderer_vk
    Freetype::Freetype
    SDL2::SDL2
    Threads::Threads
    glm::glm
)

add_dependencies( starace shader model )

target_sources( cooker_obj
    PRIVATE
    cooker_obj.cpp
    obj.hpp
)
set_target_properties( cooker_obj PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
)


