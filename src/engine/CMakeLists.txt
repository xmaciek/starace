find_package( SDL2 REQUIRED )
find_package( glm REQUIRED )

add_library( engine STATIC )
target_sources( engine
    PRIVATE
    async_io.cpp
    engine.cpp
    fps_meter.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/async_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/fps_meter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/render_context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/update_context.hpp
)

set_target_properties( engine PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

set_error_checks( engine -Wno-missing-field-initializers )
set_cppver( engine 20 )

target_link_libraries( engine
    audio
    renderer_vk
    shared
    glm::glm
    SDL2::SDL2
    Tracy::Client
)


