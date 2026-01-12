add_library( unicode STATIC )
set_vs_directory( unicode "libs" )

target_sources( unicode
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/unicode/unicode.hpp
    PRIVATE
    unicode.cpp
)

target_link_libraries( unicode
    cxx::flags
)

set_target_properties( unicode PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

