add_library( extra STATIC )

target_sources( extra
    PRIVATE
    obj.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/args.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/dds.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/fnta.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/obj.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/pak.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/tga.hpp
)
set_vs_directory( extra "libs" )

set_target_properties( extra PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( extra
    Tracy::Client
    cxx::flags
)
