find_package( Vulkan REQUIRED )

add_library( renderer_vk STATIC )
target_sources( renderer_vk
    PRIVATE
    buffer_pool.cpp
    buffer_pool.hpp
    buffer_vk.cpp
    buffer_vk.hpp
    command_pool.cpp
    command_pool.hpp
    debug_messanger.cpp
    debug_messanger.hpp
    descriptor_set.cpp
    descriptor_set.hpp
    pipeline_vk.cpp
    pipeline_vk.hpp
    render_target.cpp
    render_target.hpp
    renderer_vk.cpp
    renderer_vk.hpp
    renderpass.cpp
    renderpass.hpp
    shader.cpp
    shader.hpp
    swapchain.cpp
    swapchain.hpp
    texture_vk.cpp
    texture_vk.hpp
    utils_vk.cpp
    utils_vk.hpp
)

target_compile_options( renderer_vk PRIVATE -Werror -Wall -Wextra -Wno-missing-field-initializers  )

set_target_properties( renderer_vk PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED 17
)

target_link_libraries( renderer_vk
    renderer
    Vulkan::Vulkan
)
