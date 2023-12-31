find_package( Threads REQUIRED )

add_subdirectory( cooker )

add_subdirectory( audio )
add_subdirectory( config )
add_subdirectory( engine )
add_subdirectory( extra )
add_subdirectory( renderer )
add_subdirectory( shared )
add_subdirectory( ui )

set( STARACE_SOURCES
    action_state_tracker.cpp
    action_state_tracker.hpp
    autoaim.hpp
    autolerp.hpp
    bullet.cpp
    bullet.hpp
    circle.hpp
    colors.hpp
    enemy.cpp
    enemy.hpp
    explosion.cpp
    explosion.hpp
    game.cpp
    game.hpp
    game_action.hpp
    game_callbacks.hpp
    game_options.hpp
    game_pipeline.hpp
    hud.cpp
    hud.hpp
    jet.cpp
    jet.hpp
    main.cpp
    map_create_info.hpp
    mesh.cpp
    mesh.hpp
    model.cpp
    model.hpp
    model_proto.hpp
    progressbar.cpp
    progressbar.hpp
    saobject.cpp
    saobject.hpp
    shield.cpp
    shield.hpp
    skybox.cpp
    skybox.hpp
    space_dust.cpp
    space_dust.hpp
    speed_meter.cpp
    speed_meter.hpp
    targeting.cpp
    targeting.hpp
    texture.cpp
    texture.hpp
    thruster.cpp
    thruster.hpp
    ui_glow.cpp
    ui_glow.hpp
    ui_options_models.hpp
    ui_rings.cpp
    ui_rings.hpp
    ui_screen.cpp
    ui_screen.hpp
    units.hpp
    utils.cpp
    utils.hpp
)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( bin_name "starace" )
add_executable( ${bin_name} ${STARACE_SOURCES} )
set_cppver( ${bin_name} 20 )

target_link_libraries( ${bin_name}
    cxx::flags
    config
    engine
    extra
    ui
    SDL2::SDL2
    SDL2::main
    Threads::Threads
    glm::glm
)

