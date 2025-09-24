function( cook_atlas )
    cmake_parse_arguments( COOK_ATLAS "REMAP" "SRC;DST;PACK" "" ${ARGN} )
    if ( NOT DEFINED COOK_ATLAS_SRC )
        message( FATAL_ERROR "SRC not provided" )
    endif()
    if ( NOT DEFINED COOK_ATLAS_DST )
        message( FATAL_ERROR "DST file not provided" )
    endif()
    if ( NOT DEFINED COOK_ATLAS_PACK )
        set( COOK_ATLAS_PACK ${DEFAULT_PACK} )
    endif()

    if ( COOK_ATLAS_REMAP )
        set( COOK_ATLAS_REMAP "--remap" )
    else()
        set( COOK_ATLAS_REMAP "" )
    endif()

    add_custom_target( atlas.${COOK_ATLAS_DST}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
    )

    set_vs_directory( atlas.${COOK_ATLAS_DST} "assets/misc" )
    pak_file_cooked( ${COOK_ATLAS_PACK} "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}" atlas.${COOK_ATLAS_DST} )

    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
        DEPENDS cooker_atlas "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_ATLAS_SRC}"
        COMMAND cooker_atlas
            -i "${CMAKE_CURRENT_SOURCE_DIR}/${COOK_ATLAS_SRC}"
            -o "${CMAKE_CURRENT_BINARY_DIR}/${COOK_ATLAS_DST}"
            ${COOK_ATLAS_REMAP}
    )
endfunction()

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


cook_atlas( SRC "ui_atlas.txt" DST "ui_atlas.fnta" )
cook_atlas( SRC "init.txt" DST "init.fnta" PACK init )
cook_atlas( SRC "xbox_atlas.txt" DST "xbox_atlas.fnta" REMAP )
cook_atlas( SRC "ps4_atlas.txt" DST "ps4_atlas.fnta" REMAP )
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
