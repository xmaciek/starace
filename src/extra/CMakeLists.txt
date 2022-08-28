add_library( extra INTERFACE )
set_cppver( extra 20 )

target_sources( extra
    #PRIVATE
    #dds.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/extra/dds.hpp
)

set_target_properties( extra PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( extra
    INTERFACE
    cxx::flags
)
