list( APPEND sounds_list "${CMAKE_CURRENT_SOURCE_DIR}/blaster.wav" )
list( APPEND sounds_list "${CMAKE_CURRENT_SOURCE_DIR}/laser.wav" )
list( APPEND sounds_list "${CMAKE_CURRENT_SOURCE_DIR}/torpedo.wav" )
list( APPEND sounds_list "${CMAKE_CURRENT_SOURCE_DIR}/click.wav" )

target_sources( sounds PRIVATE "${sounds_list}" )
