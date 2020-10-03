find_package( Vulkan REQUIRED )

add_library( renderer_vk STATIC )
target_sources( renderer_vk
    PRIVATE
    buffer_vk.cpp
    buffer_vk.hpp
    pipeline_vk.cpp
    pipeline_vk.hpp
    renderer_vk.cpp
    renderer_vk.hpp
    shader.cpp
    shader.hpp
)

target_compile_options( renderer_vk PRIVATE -Werror -Wall -Wextra )

set_target_properties( renderer_vk PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
)

target_link_libraries( renderer_vk
    renderer
    Vulkan::Vulkan
)
