add_library( renderer INTERFACE )
set_cppver( renderer 20 )

target_sources( renderer
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/buffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/renderer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/renderer/texture.hpp
)

set_target_properties( renderer PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( renderer
    INTERFACE
    cxx::flags
    SDL2::SDL2
)

add_subdirectory( vulkan )
