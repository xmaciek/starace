#include "sa.hpp"

#include <random>

double randomRange( double a, double b )
{
    static std::mt19937 random( std::random_device{}() );
    return ( b - a ) * static_cast<double>( random() ) / std::mt19937::max() + a;
}

double colorHalf( double col )
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}

double lengthV( const Vertex& v )
{
    return sqrt( dotProduct( v, v ) );
}

Vertex crossProduct( const Vertex& a, const Vertex& b )
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
};

double dotProduct( const Vertex& a, const Vertex& b )
{
    return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
}

void normalizeV( Vertex& v )
{
    double length = lengthV( v );
    if ( length < 0.00001 ) {
        length = 1;
    }
    v.x /= length;
    v.y /= length;
    v.z /= length;
};

double distanceV( const Vertex& v1, const Vertex& v2 )
{
    return sqrt(
        ( ( v1.x - v2.x ) * ( v1.x - v2.x ) ) + ( ( v1.y - v2.y ) * ( v1.y - v2.y ) ) + ( ( v1.z - v2.z ) * ( v1.z - v2.z ) ) );
};
