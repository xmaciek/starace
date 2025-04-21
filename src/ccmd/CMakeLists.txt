add_library( ccmd STATIC )

target_sources( ccmd PRIVATE
    ccmd.cpp
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/public/ccmd/ccmd.hpp"
)

set_vs_directory( ccmd "libs" )

set_target_properties( ccmd PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)
