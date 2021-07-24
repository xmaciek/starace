#include "utils.hpp"

#include <random>

float randomRange( float a, float b )
{
    using Rand = std::mt19937;
    static constexpr float max = (float)Rand::max();
    thread_local Rand random( std::random_device{}() );
    const uint64_t r = random();
    return ( b - a ) * static_cast<float>( r ) / max + a;
}

double colorHalf( double col )
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}
