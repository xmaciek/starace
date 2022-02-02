#pragma once

static constexpr long double meter = 0.02415;
static constexpr long double kmph = ( 1.0 / 3600.0 ) * 1000.0 * meter;

constexpr float operator ""_kmph( long double v ) noexcept
{
    return v * kmph;
}

constexpr float operator ""_kmph( unsigned long long v ) noexcept
{
    return static_cast<long double>( v ) * kmph;
}

constexpr float value2kmph( float v ) noexcept
{
    return v / kmph;
}

constexpr float operator ""_m( long double v ) noexcept
{
    return v * meter;
}

constexpr static float operator ""_deg ( long double f ) noexcept
{
    return static_cast<float>( f * 0.01745329251994329576923690768489 );
}
