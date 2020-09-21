#include "quaternion.hpp"

#include <cmath>

Quaternion::Quaternion( const Vertex& v, float w )
{
    m_x = v.x;
    m_y = v.y;
    m_z = v.z;
    m_w = w;
}

void Quaternion::createFromAngles( double x, double y, double z, double deg )
{
    double angle = deg * PI / 180;
    double result = std::sin( angle / 2.0 );
    m_w = std::cos( angle / 2.0 );

    m_x = x * result;
    m_y = y * result;
    m_z = z * result;

    angle = sqrt( m_w * m_w + m_x * m_x + m_z * m_z + m_y * m_y );
    if ( angle == 0 ) {
        angle = 1;
    }
    m_x /= angle;
    m_y /= angle;
    m_z /= angle;
    m_w /= angle;
}

void Quaternion::createMatrix( float* matrix ) const
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

void Quaternion::inverse()
{
    m_x = -m_x;
    m_y = -m_y;
    m_z = -m_z;
    m_w = -m_w;
}

void Quaternion::conjugate()
{
    m_x = -m_x;
    m_y = -m_y;
    m_z = -m_z;
}

void Quaternion::normalize()
{
    double len = sqrt( m_w * m_w + m_x * m_x + m_y * m_y + m_z * m_z );
    if ( len == 0 ) {
        len = 1;
    }
    m_w /= len;
    m_x /= len;
    m_y /= len;
    m_z /= len;
}

Vertex Quaternion::toVector() const
{
    return Vertex{ m_x, m_y, m_z };
}

void Quaternion::rotateVector( Vertex& v )
{
    normalize();
    Quaternion V( v, 0 );
    Quaternion con = *this;
    con.conjugate();
    Quaternion result = ( con * V * *this );
    v = result.toVector();
}

Quaternion Quaternion::operator*( const Quaternion& q ) const
{
    Quaternion R;

    R.m_w = m_w * q.m_w - m_x * q.m_x - m_y * q.m_y - m_z * q.m_z;
    R.m_x = m_w * q.m_x + m_x * q.m_w + m_y * q.m_z - m_z * q.m_y;
    R.m_y = m_w * q.m_y + m_y * q.m_w + m_z * q.m_x - m_x * q.m_z;
    R.m_z = m_w * q.m_z + m_z * q.m_w + m_x * q.m_y - m_y * q.m_x;

    return R;
}
