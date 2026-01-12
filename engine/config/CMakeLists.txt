add_library( config STATIC )

target_sources( config
    PRIVATE
    config.cpp
    cfg.hpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/config/config.hpp
)
set_vs_directory( config "libs" )

set_target_properties( config PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( config
    cxx::flags
    unicode
)
