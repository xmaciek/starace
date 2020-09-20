#pragma once

#include "circle.hpp"
#include "sa.hpp"

class Thruster {
private:
    GLdouble m_length = 0.0;
    GLdouble m_lengthShorter = 0.0;
    GLdouble m_len = 0.0;
    GLuint m_wiggle = 0;
    GLfloat m_color[ 4 ][ 4 ]{};
    Circle m_inner;
    Circle m_outer;

public:
    Thruster( GLdouble length, GLdouble radius );

    void drawAt( GLdouble x, GLdouble y, GLdouble z );
    void update();
    void setColor( GLuint num, GLfloat* colorData );
    void setLength( GLdouble newLength );
};
