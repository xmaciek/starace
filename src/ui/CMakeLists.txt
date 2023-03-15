add_library( ui STATIC )
set_cppver( ui 20 )

target_sources( ui
    PRIVATE
    scroll_index.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/scroll_index.hpp
)
set_vs_directory( ui "libs" )

set_target_properties( ui PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( ui
    shared
    cxx::flags
    Tracy::Client
)
