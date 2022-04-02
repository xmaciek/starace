add_library( config STATIC )
set_cppver( config 20 )

target_sources( config
    PRIVATE
    config.cpp
    cfg.hpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/config/config.hpp
)

set_target_properties( config PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)
