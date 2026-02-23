add_library( input STATIC )

target_sources( input
    PRIVATE
    remapper.cpp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/actuator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/action.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/mouse_event.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/remapper.hpp
)
set_vs_directory( input "libs" )

set_target_properties( input PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( input
    SDL2::SDL2
    cxx::flags
    extra
    math
    profiler
    unicode
)
