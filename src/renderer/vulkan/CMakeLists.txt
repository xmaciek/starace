add_library( renderer_vk STATIC )
set_cppver( renderer_vk 20 )
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
    device_memory.cpp
    device_memory.hpp
    frame.hpp
    image.cpp
    image.hpp
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
    uniform.cpp
    uniform.hpp
    utils_vk.cpp
    utils_vk.hpp
    vk.hpp
    vk.cpp
    vk.def
)

target_link_libraries( renderer_vk
    cxx::flags
    renderer
    shared
    Vulkan::Headers
    Tracy::Client
)

target_compile_definitions( renderer_vk
    PRIVATE
    VK_NO_PROTOTYPES=1
)