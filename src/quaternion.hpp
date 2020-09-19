#ifndef SA_QUATERNION_H
#define SA_QUATERNION_H

#include "sa.hpp"

class Quaternion {
public:
    ~Quaternion() = default;
    Quaternion() = default;
    Quaternion( const Vertex& v, GLfloat w );
    void Conjugate();
    void Inverse();
    void Normalise();
    void CreateMatrix( GLfloat* mat );
    void CreateFromAngles( const GLdouble& X, const GLdouble& Y, const GLdouble& Z, const GLdouble& deg );
    Quaternion operator*( const Quaternion& Q ) const;
    Quaternion& operator=( const Quaternion& Q );
    void RotateVector( Vertex& v );
    Vertex GetVector() const;

private:
    GLdouble x = 0.0;
    GLdouble y = 0.0;
    GLdouble z = 0.0;
    GLdouble w = 1.0;
};

#endif
