#pragma once

constexpr float operator ""_kmps( long double v ) noexcept
{
    return static_cast<float>( v / 270.0 );
}

constexpr float operator ""_kmps( unsigned long long v ) noexcept
{
    return static_cast<long double>( v ) / 270.0;
}

constexpr float value2kmps( float v ) noexcept
{
    return v * 270.0f;
}
