add_library( input SHARED )

target_sources( input
    PRIVATE
    dummy.cpp
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/actuator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/input/action.hpp
)
set_vs_directory( input "libs" )

set_target_properties( input PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( input
    profiler
    cxx::flags
    SDL2::SDL2
)
