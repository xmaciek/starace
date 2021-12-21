if ( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/freetype2/include/ft2build.h" )
    add_library( Freetype::Freetype STATIC IMPORTED GLOBAL )
    set_target_properties( Freetype::Freetype PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/freetype2/include"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/freetype2/release static/vs2015-2022/win64/freetype.lib"
    )
else()
    add_library( Freetype::Freetype SHARED IMPORTED GLOBAL )
    set_target_properties( Freetype::Freetype PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/usr/include/freetype2"
        IMPORTED_LOCATION "/usr/lib/libfreetype.so"
    )
endif()

if ( NOT TARGET Freetype::Freetype )
    message( FATAL_ERROR "Freetype 2 not found" )
endif()
