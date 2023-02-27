list( APPEND sound_list "${CMAKE_CURRENT_SOURCE_DIR}/blaster.wav" )
list( APPEND sound_list "${CMAKE_CURRENT_SOURCE_DIR}/laser.wav" )
list( APPEND sound_list "${CMAKE_CURRENT_SOURCE_DIR}/torpedo.wav" )
list( APPEND sound_list "${CMAKE_CURRENT_SOURCE_DIR}/click.wav" )

pack_tar( ARCHIVE "sounds.tar" FILES "${sound_list}" )
