add_library( input STATIC )

target_sources( input
    PRIVATE
    remapper.cpp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/actuator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/action.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/remapper.hpp
)
set_vs_directory( input "libs" )

set_target_properties( input PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( input
    profiler
    extra
    unicode
    cxx::flags
    SDL2::SDL2
)
