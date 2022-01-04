find_package( Threads REQUIRED )
find_package( SDL2 REQUIRED )

add_subdirectory( audio )
add_subdirectory( renderer )
add_subdirectory( shared )
add_subdirectory( engine )

set( STARACE_SOURCES
    anchor.hpp
    bullet.cpp
    bullet.hpp
    button.cpp
    button.hpp
    circle.hpp
    colors.hpp
    constants.hpp
    enemy.cpp
    enemy.hpp
    font.cpp
    font.hpp
    game.cpp
    game.hpp
    game_action.hpp
    game_pipeline.hpp
    game_rendering.cpp
    hud.cpp
    hud.hpp
    hudbar.cpp
    hudbar.hpp
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
    progressbar.cpp
    progressbar.hpp
    reactor.cpp
    reactor.hpp
    saobject.cpp
    saobject.hpp
    screen_mission_select.cpp
    screen_mission_select.hpp
    screen_title.cpp
    screen_title.hpp
    screen_pause.cpp
    screen_pause.hpp
    screen_win_loose.cpp
    screen_win_loose.hpp
    shield.cpp
    shield.hpp
    speed_meter.cpp
    speed_meter.hpp
    targeting.cpp
    targeting.hpp
    texture.cpp
    texture.hpp
    tga.hpp
    thruster.cpp
    thruster.hpp
    ui_image.cpp
    ui_image.hpp
    ui_rings.cpp
    ui_rings.hpp
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
