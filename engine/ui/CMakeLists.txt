add_library( ui STATIC )

target_sources( ui
    PRIVATE
    animframe.cpp
    animframe.hpp
    button.cpp
    button.hpp
    combobox.cpp
    combobox.hpp
    data_model.cpp
    font.cpp
    font_map.cpp
    footer.cpp
    footer.hpp
    glow.cpp
    glow.hpp
    image.cpp
    image.hpp
    label.cpp
    lockit.cpp
    message_box.cpp
    nineslice.cpp
    progressbar.cpp
    progressbar.hpp
    property.cpp
    screen.cpp
    scroll_index.cpp
    spinbox.cpp
    spinbox.hpp
    widget.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/anchor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/data_model.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/font.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/font_map.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/input.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/label.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/lockit.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/message_box.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/nineslice.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/property.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/screen.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/scroll_index.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/sprite.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/tab_order.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/var.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/widget.hpp

)
set_vs_directory( ui "libs" )

set_target_properties( ui PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

target_link_libraries( ui
    cxx::flags
    config
    engine #for math.hpp
    input
    profiler
    renderer
    shared
)
