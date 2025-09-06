find_package( Threads REQUIRED )

add_subdirectory( audio )
add_subdirectory( ccmd )
add_subdirectory( config )
add_subdirectory( cooker )
add_subdirectory( engine )
add_subdirectory( extra )
add_subdirectory( input )
add_subdirectory( platform )
add_subdirectory( profiler )
add_subdirectory( renderer )
add_subdirectory( shared )
add_subdirectory( ui )
add_subdirectory( unicode )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

add_executable( starace WIN32 )
set_target_properties( starace PROPERTIES OUTPUT_NAME "starace" )
target_sources( starace PRIVATE
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
    game_options.cpp
    game_options.hpp
    game_pipeline.hpp
    game_scene.cpp
    game_scene.hpp
    main.cpp
    map_create_info.hpp
    menu_scene.hpp
    menu_scene.cpp
    mesh.cpp
    mesh.hpp
    model.cpp
    model.hpp
    model_proto.hpp
    player.cpp
    player.hpp
    render_context.hpp
    saobject.cpp
    saobject.hpp
    signal.hpp
    skybox.cpp
    skybox.hpp
    space_dust.cpp
    space_dust.hpp
    targeting.cpp
    targeting.hpp
    texture.cpp
    texture.hpp
    ui_options_models.hpp
    units.hpp
    update_context.hpp
    utils.cpp
    utils.hpp
)

option( STRIP_SYMBOLS "Whether to enable symbol stripping" FALSE )
if ( ${STRIP_SYMBOLS} )
    # TODO: test target property LINKER_TYPE
    if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
        set_target_properties( starace PROPERTIES LINK_FLAGS -s )
    endif()
endif()

target_link_libraries( starace
    cxx::flags
    config
    engine
    extra
    ui
    profiler
    SDL2::SDL2
    SDL2::main
    Threads::Threads
    glm::glm
)

