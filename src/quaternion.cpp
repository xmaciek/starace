#include "quaternion.hpp"

Quaternion::Quaternion( const Vertex& v, GLfloat W )
{
    m_x = v.x;
    m_y = v.y;
    m_z = v.z;
    m_w = W;
}

void Quaternion::CreateFromAngles( const GLdouble& X, const GLdouble& Y, const GLdouble& Z, const GLdouble& deg )
{
    GLdouble angle = deg * PI / 180;
    GLdouble result = sin( angle / 2.0 );
    m_w = cos( angle / 2.0 );

    m_x = X * result;
    m_y = Y * result;
    m_z = Z * result;

    angle = sqrt( m_w * m_w + m_x * m_x + m_z * m_z + m_y * m_y );
    if ( angle == 0 ) {
        angle = 1;
    }
    m_x /= angle;
    m_y /= angle;
    m_z /= angle;
    m_w /= angle;
}

void Quaternion::CreateMatrix( GLfloat* matrix ) const
{
    matrix[ 0 ] = 1.0 - 2.0 * ( m_y * m_y + m_z * m_z );
    matrix[ 1 ] = 2.0 * ( m_x * m_y - m_w * m_z );
    matrix[ 2 ] = 2.0 * ( m_x * m_z + m_w * m_y );
    matrix[ 3 ] = 0;

    matrix[ 4 ] = 2.0 * ( m_x * m_y + m_w * m_z );
    matrix[ 5 ] = 1.0 - 2.0 * ( m_x * m_x + m_z * m_z );
    matrix[ 6 ] = 2.0 * ( m_y * m_z - m_w * m_x );
    matrix[ 7 ] = 0;

    matrix[ 8 ] = 2.0 * ( m_x * m_z - m_w * m_y );
    matrix[ 9 ] = 2.0 * ( m_y * m_z + m_w * m_x );
    matrix[ 10 ] = 1.0 - 2.0 * ( m_x * m_x + m_y * m_y );
    matrix[ 11 ] = 0;

    matrix[ 12 ] = 0;
    matrix[ 13 ] = 0;
    matrix[ 14 ] = 0;
    matrix[ 15 ] = 1.0;
}

void Quaternion::Inverse()
{
    m_x = -m_x;
    m_y = -m_y;
    m_z = -m_z;
    m_w = -m_w;
}

void Quaternion::Conjugate()
{
    m_x = -m_x;
    m_y = -m_y;
    m_z = -m_z;
}

void Quaternion::Normalise()
{
    GLdouble len = sqrt( m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z );
    if ( len == 0 ) {
        len = 1;
    }
    m_w /= len;
    m_x /= len;
    m_y /= len;
    m_z /= len;
}

Vertex Quaternion::GetVector() const
{
    Vertex v;
    v.x = m_x;
    v.y = m_y;
    v.z = m_z;
    return v;
}

void Quaternion::RotateVector( Vertex& v )
{
    Normalise();
    Quaternion V( v, 0 );
    Quaternion con = *this;
    con.Conjugate();
    Quaternion result = ( con * V * *this );
    v = result.GetVector();
}

Quaternion Quaternion::operator*( const Quaternion& Q ) const
{
    Quaternion R;

    R.m_w = m_w * Q.m_w - m_x * Q.m_x - m_y * Q.m_y - m_z * Q.m_z;
    R.m_x = m_w * Q.m_x + m_x * Q.m_w + m_y * Q.m_z - m_z * Q.m_y;
    R.m_y = m_w * Q.m_y + m_y * Q.m_w + m_z * Q.m_x - m_x * Q.m_z;
    R.m_z = m_w * Q.m_z + m_z * Q.m_w + m_x * Q.m_y - m_y * Q.m_x;

    return R;
}
