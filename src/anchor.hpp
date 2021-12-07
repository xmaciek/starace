#pragma once

#include <cstdint>
#include <type_traits>

constexpr uint32_t operator ""_bit( unsigned long long n ) noexcept
{
    return 1ull << n;
}

enum class Anchor : uint32_t {
    fLeft = 0_bit,
    fCenter = 1_bit,
    fRight = 2_bit,
    fBottom = 3_bit,
    fMiddle = 4_bit,
    fTop = 5_bit,
};

constexpr Anchor operator | ( Anchor a, Anchor b ) noexcept
{
    using T = std::underlying_type_t<Anchor>;
    return static_cast<Anchor>( static_cast<T>( a ) | static_cast<T>( b ) );
}

constexpr Anchor operator & ( Anchor a, Anchor b ) noexcept
{
    using T = std::underlying_type_t<Anchor>;
    return static_cast<Anchor>( static_cast<T>( a ) & static_cast<T>( b ) );
}

constexpr bool operator && ( Anchor a, Anchor b ) noexcept
{
    return ( a & b ) == b;
}
