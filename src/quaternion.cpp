#include "quaternion.hpp"

Quaternion::Quaternion()
{
    x = 0;
    y = 0;
    z = 0;
    w = 1;
}

Quaternion::Quaternion( Vertex v, GLfloat W )
{
    x = v.x;
    y = v.y;
    z = v.z;
    w = W;
}

Quaternion::~Quaternion() { }

void Quaternion::CreateFromAngles( const GLdouble& X, const GLdouble& Y, const GLdouble& Z, const GLdouble& deg )
{
    GLdouble angle = deg * PI / 180;
    GLdouble result = sin( angle / 2.0 );
    w = cos( angle / 2.0 );

    x = X * result;
    y = Y * result;
    z = Z * result;

    angle = sqrt( w * w + x * x + z * z + y * y );
    if ( angle == 0 )
        angle = 1;
    x /= angle;
    y /= angle;
    z /= angle;
    w /= angle;
}

void Quaternion::CreateMatrix( GLfloat* matrix )
{
    matrix[ 0 ] = 1.0 - 2.0 * ( y * y + z * z );
    matrix[ 1 ] = 2.0 * ( x * y - w * z );
    matrix[ 2 ] = 2.0 * ( x * z + w * y );
    matrix[ 3 ] = 0;

    matrix[ 4 ] = 2.0 * ( x * y + w * z );
    matrix[ 5 ] = 1.0 - 2.0 * ( x * x + z * z );
    matrix[ 6 ] = 2.0 * ( y * z - w * x );
    matrix[ 7 ] = 0;

    matrix[ 8 ] = 2.0 * ( x * z - w * y );
    matrix[ 9 ] = 2.0 * ( y * z + w * x );
    matrix[ 10 ] = 1.0 - 2.0 * ( x * x + y * y );
    matrix[ 11 ] = 0;

    matrix[ 12 ] = 0;
    matrix[ 13 ] = 0;
    matrix[ 14 ] = 0;
    matrix[ 15 ] = 1.0;
}

void Quaternion::Inverse()
{
    x = -x;
    y = -y;
    z = -z;
    w = -w;
}

void Quaternion::Conjugate()
{
    x = -x;
    y = -y;
    z = -z;
}

void Quaternion::Normalise()
{
    GLdouble len = sqrt( w * w + x * x + y * y + z * z );
    if ( len == 0 ) {
        len = 1;
    }
    w /= len;
    x /= len;
    y /= len;
    z /= len;
}

Vertex Quaternion::GetVector()
{
    Vertex v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

void Quaternion::RotateVector( Vertex& v )
{
    Normalise();
    Quaternion V( v, 0 );
    //   V.Normalise();
    Quaternion con = *this;
    con.Conjugate();
    Quaternion result = ( con * V * *this );
    //   result.Normalise();
    v = result.GetVector();
}

Quaternion Quaternion::operator*( const Quaternion& Q )
{
    Quaternion R;

    R.w = w * Q.w - x * Q.x - y * Q.y - z * Q.z;
    R.x = w * Q.x + x * Q.w + y * Q.z - z * Q.y;
    R.y = w * Q.y + y * Q.w + z * Q.x - x * Q.z;
    R.z = w * Q.z + z * Q.w + x * Q.y - y * Q.x;

    return R;
}

Quaternion Quaternion::operator=( const Quaternion& Q )
{
    x = Q.x;
    y = Q.y;
    z = Q.z;
    w = Q.w;
    return *this;
}
