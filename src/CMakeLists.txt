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
    tga.hpp
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

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

function( setup_game bin_name renderer_lib )
    add_executable( ${bin_name} ${STARACE_SOURCES} ${STARACE_HEADERS} )
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
        ${renderer_lib}
        Freetype::Freetype
        SDL2::SDL2
        Threads::Threads
        glm::glm
    )

endfunction()

setup_game( starace_gl renderer_gl )
setup_game( starace_vk renderer_vk )
add_dependencies( starace_vk shader )
