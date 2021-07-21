#include "utils.hpp"

#include <random>

float randomRange( float a, float b )
{
    thread_local std::mt19937 random( std::random_device{}() );
    const uint64_t r = random();
    return ( b - a ) * static_cast<float>( r ) / random.max() + a;
}

double colorHalf( double col )
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}
