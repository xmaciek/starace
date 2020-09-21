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
