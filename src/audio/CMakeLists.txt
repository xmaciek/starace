add_library( audio STATIC )

target_sources( audio
    PRIVATE
    audio.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/audio/audio.hpp
)
set_vs_directory( audio "libs" )

set_target_properties( audio PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( audio
    shared
    profiler
    platform
    cxx::flags
    SDL2::SDL2
)
