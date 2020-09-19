#include "sa.hpp"

#include <random>

GLdouble random_range( GLdouble a, GLdouble b )
{
    static std::mt19937 random( std::random_device{}() );
    return ( b - a ) * static_cast<GLdouble>( random() ) / std::mt19937::max() + a;
}

GLfloat colorhalf( GLfloat col )
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}

GLdouble length_v( const Vertex& v )
{
    return sqrt( dot_product( v, v ) );
}

Vertex cross_product( const Vertex& a, const Vertex& b )
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
};

GLdouble dot_product( const Vertex& a, const Vertex& b )
{
    return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
}

void normalise_v( Vertex& v )
{
    GLdouble length = length_v( v );
    if ( length < 0.00001 ) {
        length = 1;
    }
    v.x /= length;
    v.y /= length;
    v.z /= length;
};

GLdouble distance_v( const Vertex& v1, const Vertex& v2 )
{
    return sqrt(
        ( ( v1.x - v2.x ) * ( v1.x - v2.x ) ) + ( ( v1.y - v2.y ) * ( v1.y - v2.y ) ) + ( ( v1.z - v2.z ) * ( v1.z - v2.z ) ) );
};
