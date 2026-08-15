#pragma once

#include <math.hpp>

static constexpr long double meter = 0.02415;
static constexpr long double kmph = 3.6;
static constexpr long double mph = 2.23693629;
static constexpr long double knot = 1.94384;

constexpr float operator ""_kmph( long double v ) noexcept
{
    return static_cast<float>( ( meter * v ) / kmph );
}

constexpr float operator ""_kmph( unsigned long long v ) noexcept
{
    return static_cast<float>( ( meter * static_cast<long double>( v ) ) / kmph );
}

constexpr float value2kmph( float v ) noexcept
{
    return static_cast<float>( ( v / meter ) * kmph );
}

constexpr float operator ""_m( long double v ) noexcept
{
    return static_cast<float>( v * meter );
}

constexpr static float operator ""_deg ( unsigned long long f ) noexcept
{
    return static_cast<float>( static_cast<float>( f ) * math::rad );
}

constexpr static float operator ""_deg ( long double f ) noexcept
{
    return static_cast<float>( f * math::rad );
}

constexpr static float operator ""_kN ( long double f ) noexcept
{
    return static_cast<float>( f * 1000.0l );
}

constexpr static float operator ""_kN ( unsigned long long f ) noexcept
{
    return static_cast<float>( f * 1000ull );
}

constexpr static float operator ""_kg ( unsigned long long f ) noexcept
{
    return static_cast<float>( f );
}
