add_library( renderer_vk STATIC )
set_vs_directory( renderer_vk "libs" )

target_sources( renderer_vk
    PRIVATE
    buffer_vk.cpp
    buffer_vk.hpp
    command_pool.cpp
    command_pool.hpp
    debug_messanger.cpp
    debug_messanger.hpp
    descriptor_set.cpp
    descriptor_set.hpp
    device.cpp
    device.hpp
    device_memory.cpp
    device_memory.hpp
    frame.hpp
    image.cpp
    image.hpp
    instance.cpp
    instance.hpp
    pipeline_vk.cpp
    pipeline_vk.hpp
    queue_manager.cpp
    queue_manager.hpp
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
    uniform.cpp
    uniform.hpp
    utils_vk.cpp
    utils_vk.hpp
    vk.def
    vk.hpp
    wishlist.hpp
)

target_link_libraries( renderer_vk
    cxx::flags
    platform
    renderer
    shared
    profiler
    Vulkan::Headers
)

target_compile_definitions( renderer_vk
    PRIVATE
    ENABLE_VULKAN_VALIDATION=1
    VK_NO_PROTOTYPES=1
)

