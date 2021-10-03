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

set_error_checks( renderer_vk -Wno-missing-field-initializers )
set_cppver( renderer_vk 20 )

target_link_libraries( renderer_vk
    renderer
    Vulkan::Vulkan
)
