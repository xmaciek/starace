list( APPEND ui_list "${CMAKE_CURRENT_SOURCE_DIR}/mainmenu.ui" )
list( APPEND ui_list "${CMAKE_CURRENT_SOURCE_DIR}/missionselect.ui" )
list( APPEND ui_list "${CMAKE_CURRENT_SOURCE_DIR}/customize.ui" )
list( APPEND ui_list "${CMAKE_CURRENT_SOURCE_DIR}/settings.ui" )
list( APPEND ui_list "${CMAKE_CURRENT_SOURCE_DIR}/pause.ui" )
list( APPEND ui_list "${CMAKE_CURRENT_SOURCE_DIR}/result.ui" )

pack_tar( ARCHIVE "ui.tar" FILES "${ui_list}" )
