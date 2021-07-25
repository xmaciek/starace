#pragma once

float randomRange( float a, float b );
double colorHalf( double col );

constexpr float lerp( float a, float b, float n ) noexcept
{
    const float range = b - a;
    return a + range * n;
}
