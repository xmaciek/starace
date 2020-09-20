#pragma once
#include "sa.hpp"

class Quaternion {
private:
    GLdouble m_x = 0.0;
    GLdouble m_y = 0.0;
    GLdouble m_z = 0.0;
    GLdouble m_w = 1.0;

public:
    Quaternion() = default;
    Quaternion( const Vertex& v, GLfloat w );
    Quaternion operator*( const Quaternion& ) const;

    Vertex toVector() const;
    void conjugate();
    void createFromAngles( GLdouble x, GLdouble y, GLdouble z, GLdouble deg );
    void createMatrix( GLfloat* mat ) const;
    void inverse();
    void normalize();
    void rotateVector( Vertex& );
};
