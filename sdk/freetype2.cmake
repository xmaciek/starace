function( findFreeType2 dir subdir )
    if ( NOT WIN32 )
        return()
    endif()
    if ( TARGET Freetype::Freetype )
        return()
    endif()

    if ( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include/ft2build.h" )
        return()
    endif()
    add_library( Freetype::Freetype STATIC IMPORTED GLOBAL )
    set_target_properties( Freetype::Freetype PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include"
        IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/release static/${subdir}/win64/freetype.lib"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/release static/${subdir}/win64/freetype.lib"
    )
endfunction()

findFreeType2( "freetype-windows-binaries-master" "vs2015-2022" )

if ( UNIX )
    if ( NOT EXISTS "/usr/include/freetype2/ft2build.h" )
        return()
    endif()
    add_library( Freetype::Freetype SHARED IMPORTED GLOBAL )
    set_target_properties( Freetype::Freetype PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "/usr/include/freetype2"
        IMPORTED_LOCATION "/usr/lib/libfreetype.so"
    )
endif()

if ( TARGET Freetype::Freetype )
    return()
endif()

if ( WIN32 )
    set( ft2_download_link "https://freetype.org/download.html" )
    set( ft2_unpack_dir "${CMAKE_CURRENT_SOURCE_DIR}/freetype-windows-binaries-master" )
    message( FATAL_ERROR "Freetype 2 not found, see if you can download ft2 build variant for visual studio from ${ft2_download_link} and unpack it into \"${ft2_unpack_dir}\"" )
else()
    message( FATAL_ERROR "Freetype 2 not found" )
endif()
