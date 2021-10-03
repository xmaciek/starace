find_package( SDL2 REQUIRED )

add_library( audio STATIC )
target_sources( audio
    PRIVATE
    audio.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/audio/audio.hpp
)

set_error_checks( audio )

set_target_properties( audio PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( audio
    SDL2::SDL2
)
