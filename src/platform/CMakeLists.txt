add_library( platform STATIC )
set_vs_directory( platform "libs" )

target_sources( platform
    PRIVATE
    utils.cpp

    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/public/platform/utils.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/public/platform/windows.hpp"
)

set_target_properties( platform PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( platform
    cxx::flags
    SDL2::SDL2
)
