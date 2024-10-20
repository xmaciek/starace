find_package( Threads REQUIRED )

add_subdirectory( cooker )

add_subdirectory( audio )
add_subdirectory( config )
add_subdirectory( engine )
add_subdirectory( extra )
add_subdirectory( platform )
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
    game_scene.cpp
    game_scene.hpp
    jet.cpp
    jet.hpp
    main.cpp
    map_create_info.hpp
    mesh.cpp
    mesh.hpp
    model.cpp
    model.hpp
    model_proto.hpp
    render_context.hpp
    saobject.cpp
    saobject.hpp
    skybox.cpp
    skybox.hpp
    space_dust.cpp
    space_dust.hpp
    targeting.cpp
    targeting.hpp
    texture.cpp
    texture.hpp
    ui_glow.cpp
    ui_glow.hpp
    ui_options_models.hpp
    ui_rings.cpp
    ui_rings.hpp
    units.hpp
    update_context.hpp
    utils.cpp
    utils.hpp
)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

set( bin_name "starace" )
add_executable( ${bin_name} ${STARACE_SOURCES} )

# strip
if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    set_target_properties( ${bin_name} PROPERTIES LINK_FLAGS_RELEASE -s )
endif()

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

