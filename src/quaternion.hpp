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
    Quaternion operator*( const Quaternion& Q ) const;

    Vertex GetVector() const;
    void Conjugate();
    void CreateFromAngles( const GLdouble& X, const GLdouble& Y, const GLdouble& Z, const GLdouble& deg );
    void CreateMatrix( GLfloat* mat ) const;
    void Inverse();
    void Normalise();
    void RotateVector( Vertex& v );
};
