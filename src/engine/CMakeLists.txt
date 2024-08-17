add_library( engine STATIC )

target_sources( engine
    PRIVATE
    filesystem.cpp
    engine.cpp
    controller_state.hpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/action.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/filesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/math.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/render_context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/update_context.hpp
)
set_vs_directory( engine "libs" )

set_target_properties( engine PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( engine
    cxx::flags
    audio
    extra
    renderer_vk
    shared
    glm::glm
    SDL2::SDL2
    Tracy::Client
)
