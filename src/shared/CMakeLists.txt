add_library( shared INTERFACE )
target_sources( shared
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/fixed_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/indexer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/shared/pool.hpp
)

set_target_properties( shared PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

set_cppver( shared 20 )
