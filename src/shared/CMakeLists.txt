add_library( shared INTERFACE )
set_cppver( shared 20 )

target_sources( shared
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/bit_index_iterator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/fixed_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/hash.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/indexer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/pool.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/rotary_index.hpp
)
set_vs_directory( shared "libs" )

set_target_properties( shared PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( shared
    INTERFACE
    cxx::flags
)
