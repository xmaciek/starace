add_library( engine STATIC )
find_package( Threads REQUIRED )

target_sources( engine
    PRIVATE
    filesystem.cpp
    savesystem.cpp
    engine.cpp
    controller_state.hpp
    material_setup.hpp
    material_setup.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/filesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/savesystem.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/math.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/engine/resource_map.hpp
)
set_vs_directory( engine "libs" )

set_target_properties( engine PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( engine
    cxx::flags
    Threads::Threads
    glm::glm
    SDL2::SDL2
    audio
    ccmd
    extra
    input
    profiler
    renderer_vk
    shared
    unicode
)
