find_package( SDL2 REQUIRED )

add_library( audio STATIC )
set_cppver( audio 20 )

target_sources( audio
    PRIVATE
    audio.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/audio/audio.hpp
)

set_target_properties( audio PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( audio
    cxx::flags
    Tracy::Client
    SDL2::SDL2
)
