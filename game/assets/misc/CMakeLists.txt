function( cook_callsign )
    cmake_parse_arguments( COOK_CALLSIGN "" "SRC;DST" "" ${ARGN} )
    if ( NOT DEFINED COOK_CALLSIGN_SRC )
        message( FATAL_ERROR "SRC not provided" )
    endif()
    if ( NOT DEFINED COOK_CALLSIGN_DST )
        message( FATAL_ERROR "DST file not provided" )
    endif()

    add_custom_target( callsign.${COOK_CALLSIGN_DST}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${COOK_CALLSIGN_DST}"
    )

    set_vs_directory( callsign.${COOK_CALLSIGN_DST} "assets/misc" )
    pak_file_cooked( ${DEFAULT_PACK} "${CMAKE_CURRENT_BINARY_DIR}/${COOK_CALLSIGN_DST}" callsign.${COOK_CALLSIGN_DST} )

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${COOK_CALLSIGN_DST}"
        DEPENDS cooker_callsign "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_CALLSIGN_SRC}"
        COMMAND cooker_callsign
            --src "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_CALLSIGN_SRC}"
            --dst "${CMAKE_CURRENT_BINARY_DIR}/${COOK_CALLSIGN_DST}"
    )
endfunction()


cook_atlas( SRC "ui_atlas.txt" DST "ui_atlas.atlas" TEXTURE "textures/ui_atlas.dds" )
cook_atlas( SRC "init.txt" DST "init.atlas" TEXTURE "textures/init.dds" PACK init )
cook_atlas( SRC "xbox_atlas.txt" DST "xbox_atlas.fnta" TEXTURE "textures/xbox_atlas.dds" REMAP )
cook_atlas( SRC "ps4_atlas.txt" DST "ps4_atlas.fnta" TEXTURE "textures/ps4_atlas.dds" REMAP )
cook_callsign( SRC "callsigns.txt" DST "callsigns.csg" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/nebula.map" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/darkspace.map" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/atmosphere.map" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/a2.jet" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/a3.jet" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/a4.jet" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/a5.jet" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/a6.jet" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/blaster.wpn" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/cannon.wpn" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/laser.wpn" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/rocket.wpn" )
pak_file( ${DEFAULT_PACK} "${CMAKE_CURRENT_SOURCE_DIR}/enemy.wpn" )
