option( EnableTracyProfiler "Whether to enable tracy profiler if present within sdk tree" TRUE )

set( tracy_dir "tracy-0.7.8" )

if ( ${EnableTracyProfiler} AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${tracy_dir}/Tracy.hpp" )
    add_library( tracy STATIC )
        target_sources( tracy
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/${tracy_dir}/Tracy.hpp"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/${tracy_dir}/TracyClient.cpp"
    )

    set_target_properties( tracy PROPERTIES
        INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/${tracy_dir}"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/${tracy_dir}"
    )
    target_compile_definitions( tracy
        PUBLIC
        TRACY_ENABLE=1
    )
    target_link_libraries( tracy dl )

else()
    add_library( tracy INTERFACE )
    target_sources( tracy
        PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/notracy/Tracy.hpp"
    )

    set_target_properties( tracy PROPERTIES
        INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/notracy"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/notracy"
    )
endif()

add_library( Tracy::Client ALIAS tracy )
