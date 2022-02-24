find_package( Threads REQUIRED )

add_subdirectory( audio )
add_subdirectory( renderer )
add_subdirectory( shared )
add_subdirectory( engine )

set( STARACE_SOURCES
    anchor.hpp
    autoaim.hpp
    bullet.cpp
    bullet.hpp
    button.cpp
    button.hpp
    circle.hpp
    colors.hpp
    constants.hpp
    enemy.cpp
    enemy.hpp
    explosion.cpp
    explosion.hpp
    font.cpp
    font.hpp
    game.cpp
    game.hpp
    game_action.hpp
    game_pipeline.hpp
    hud.cpp
    hud.hpp
    hudbar.cpp
    hudbar.hpp
    jet.cpp
    jet.hpp
    label.cpp
    label.hpp
    linear_atlas.hpp
    linear_atlas.cpp
    main.cpp
    model.cpp
    model.hpp
    model_proto.hpp
    obj.cpp
    obj.hpp
    progressbar.cpp
    progressbar.hpp
    reactor.cpp
    reactor.hpp
    saobject.cpp
    saobject.hpp
    screen_customize.cpp
    screen_customize.hpp
    screen_mission_select.cpp
    screen_mission_select.hpp
    screen_pause.cpp
    screen_pause.hpp
    screen_title.cpp
    screen_title.hpp
    screen_win_loose.cpp
    screen_win_loose.hpp
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
    tga.hpp
    thruster.cpp
    thruster.hpp
    ui_glow.cpp
    ui_glow.hpp
    ui_image.cpp
    ui_image.hpp
    ui_rings.cpp
    ui_rings.hpp
    units.hpp
    utils.cpp
    utils.hpp
    widget.cpp
    widget.hpp
)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( bin_name "starace" )
add_executable( ${bin_name} ${STARACE_SOURCES} )
set_cppver( ${bin_name} 20 )

target_link_libraries( ${bin_name}
    cxx::flags
    engine
    Freetype::Freetype
    SDL2::SDL2
    SDL2::main
    Threads::Threads
    glm::glm
)

target_sources( cooker_obj
    PRIVATE
    cooker_obj.cpp
    obj.hpp
)
target_link_libraries( cooker_obj cxx::flags )
set_cppver( cooker_obj 20 )

target_sources( cooker_tga
    PRIVATE
    cooker_tga.cpp
    tga.hpp
)
target_link_libraries( cooker_tga cxx::flags )
set_cppver( cooker_tga 20 )
