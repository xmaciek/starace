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

    void DrawAt( GLdouble X, GLdouble Y, GLdouble Z );
    void Update();
    void SetColor( GLuint Num, GLfloat* ColorData );
    void SetLength( GLdouble newLength );
};
