add_library( ui STATIC )

target_sources( ui
    PRIVATE
    animframe.cpp
    button.cpp
    combobox.cpp
    data_model.cpp
    font.cpp
    footer.cpp
    glow.cpp
    glow.hpp
    image.cpp
    label.cpp
    nineslice.cpp
    progressbar.cpp
    property.cpp
    remapper.cpp
    screen.cpp
    scroll_index.cpp
    spinbox.cpp
    spritegen.cpp
    widget.cpp

    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/anchor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/animframe.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/button.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/combobox.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/data_model.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/font.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/footer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/image.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/input.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/label.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/nineslice.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/pipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/progressbar.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/property.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/remapper.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/screen.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/scroll_index.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/spinbox.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/public/ui/spritegen.hpp
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
    shared
    engine
    renderer
    config
    cxx::flags
    profiler
)
