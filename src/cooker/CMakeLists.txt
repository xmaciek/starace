add_library( cooker_common INTERFACE )
target_sources( cooker_common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/public/cooker/common.hpp
)
set_vs_directory( cooker_common "libs" )
set_target_properties( cooker_common PROPERTIES
    INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/public/
)

declare_cooker( NAME cooker_atlas
    SRC cooker_atlas.cpp
    LINK config shared unicode
)
declare_cooker( NAME cooker_callsign
    SRC cooker_callsign.cpp
    LINK unicode
)
declare_cooker( NAME cooker_dds
    SRC cooker_dds.cpp cooker_dds.hpp cooker_tga.hpp
)
declare_cooker( NAME cooker_font
    SRC cooker_font.cpp cooker_dds.hpp
    LINK Freetype::Freetype
)
declare_cooker( NAME cooker_lang
    SRC cooker_lang.cpp
    LINK shared unicode
)
declare_cooker( NAME cooker_obj
    SRC cooker_obj.cpp
    LINK ccmd
)
declare_cooker( NAME cooker_pak
    SRC cooker_pak.cpp
)
