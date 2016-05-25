
set( STARACE_HEADERS
    Bullet.h
    Button.h
    Circle.h
    Enemy.h
    Font.h
    Jet.h
    Map.h
    Model.h
    Quaternion.h
    Road.h
    SA.h
    SAObject.h
    Shield.h
    Texture.h
    Thruster.h
)

set( STARACE_SOURCES
    Bullet.cpp
    Button.cpp
    Circle.cpp
    Enemy.cpp
    Font.cpp
    Jet.cpp
    main.cpp
    Map.cpp
    Model.cpp
    Quaternion.cpp
    Road.cpp
    Road_drawing.cpp
    Road_keyboard.cpp
    SAObject.cpp
    Shield.cpp
    Texture.cpp
    Thruster.cpp
    utils.cpp
)

set( STARACE_BINNAME "starace" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

add_executable( ${STARACE_BINNAME} ${STARACE_SOURCES} )
target_link_libraries( ${STARACE_BINNAME} SDL SDL_ttf SDL_mixer GL GLU )
