add_library( engine STATIC )
set_cppver( engine 20 )

target_sources( engine
    PRIVATE
    action_mapping.hpp
    action_mapping.cpp
    async_io.cpp
    async_io.cpp
    engine.cpp
    fps_meter.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/action.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/async_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/fps_meter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/math.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/render_context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/update_context.hpp
)

set_target_properties( engine PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( engine
    cxx::flags
    audio
    renderer_vk
    shared
    glm::glm
    SDL2::SDL2
    Tracy::Client
)
