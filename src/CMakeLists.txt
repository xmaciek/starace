find_package( Threads REQUIRED )

add_subdirectory( audio )
add_subdirectory( config )
add_subdirectory( renderer )
add_subdirectory( shared )
add_subdirectory( engine )
add_subdirectory( extra )

set( STARACE_SOURCES
    action_state_tracker.cpp
    action_state_tracker.hpp
    anchor.hpp
    autoaim.hpp
    bullet.cpp
    bullet.hpp
    button.cpp
    button.hpp
    chase.hpp
    circle.hpp
    colors.hpp
    enemy.cpp
    enemy.hpp
    explosion.cpp
    explosion.hpp
    font.cpp
    font.hpp
    game.cpp
    game.hpp
    game_action.hpp
    game_callbacks.hpp
    game_options.hpp
    game_pipeline.hpp
    hud.cpp
    hud.hpp
    hudbar.cpp
    hudbar.hpp
    jet.cpp
    jet.hpp
    label.cpp
    label.hpp
    linear_atlas.cpp
    linear_atlas.hpp
    main.cpp
    map_create_info.hpp
    mesh.cpp
    mesh.hpp
    model.cpp
    model.hpp
    model_proto.hpp
    nineslice.cpp
    nineslice.hpp
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
    spritegen.cpp
    spritegen.hpp
    tab_order.hpp
    targeting.cpp
    targeting.hpp
    texture.cpp
    texture.hpp
    thruster.cpp
    thruster.hpp
    ui_combobox.cpp
    ui_combobox.hpp
    ui_data_model.cpp
    ui_data_model.hpp
    ui_glow.cpp
    ui_glow.hpp
    ui_image.cpp
    ui_image.hpp
    ui_localize.hpp
    ui_options_models.hpp
    ui_property.hpp
    ui_rings.cpp
    ui_rings.hpp
    ui_screen.cpp
    ui_screen.hpp
    ui_scroll_index.cpp
    ui_scroll_index.hpp
    ui_spinbox.cpp
    ui_spinbox.hpp
    ui_var.hpp
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
    config
    extra
    Freetype::Freetype
    SDL2::SDL2
    SDL2::main
    Threads::Threads
    glm::glm
)

target_sources( cooker_obj PRIVATE cooker_obj.cpp )
target_link_libraries( cooker_obj cxx::flags extra )
set_cppver( cooker_obj 20 )

target_sources( cooker_dds PRIVATE cooker_dds.cpp )
target_link_libraries( cooker_dds cxx::flags extra )
set_cppver( cooker_dds 20 )

target_sources( cooker_tar PRIVATE cooker_tar.cpp )
target_link_libraries( cooker_tar cxx::flags extra )
set_cppver( cooker_tar 20 )
