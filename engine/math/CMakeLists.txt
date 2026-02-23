add_library( math INTERFACE )

target_sources( math
    PUBLIC
    math.hpp
)
set_vs_directory( math "libs" )

set_target_properties( math PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries( math INTERFACE
    glm::glm
)
