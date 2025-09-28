add_library( unicode INTERFACE )

target_sources( unicode
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/unicode/unicode.hpp
)
set_vs_directory( unicode "libs" )

set_target_properties( unicode PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

