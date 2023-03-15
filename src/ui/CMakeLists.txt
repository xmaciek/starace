add_library( ui STATIC )
set_cppver( ui 20 )

target_sources( ui
    PRIVATE
    data_model.cpp
    scroll_index.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/data_model.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/scroll_index.hpp
)
set_vs_directory( ui "libs" )

set_target_properties( ui PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( ui
    shared
    engine
    renderer
    cxx::flags
    Tracy::Client
)
