#pragma once

#include <engine/math.hpp>

#include <algorithm>
#include <array>
#include <memory_resource>
#include <random>
#include <string>
#include <type_traits>

using Random = std::minstd_rand;

math::vec3 project3dTo2d( const math::mat4& mvp, const math::vec3& point, const math::vec2& viewport );
bool isOnScreen( const math::vec3& point, const math::vec2& viewport );
bool isOnScreen( const math::mat4& mvp, const math::vec3& point, const math::vec2& viewport );
float randomRange( float a, float b );

constexpr double colorHalf( double col ) noexcept
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}

template <typename T>
requires std::is_integral_v<T>
char32_t* toChars( char32_t* begin, [[maybe_unused]] char32_t* end, T t )
{
    uint64_t v = 0;
    if constexpr ( std::is_signed_v<T> ) {
        if ( t < 0 ) {
            *begin++ = U'-';
            v = static_cast<uint64_t>( -t );
        }
        else {
            v = static_cast<uint64_t>( t );
        }
    }
    else {
        v = t;
    }
    char32_t* it = begin;
    do {
        *it++ = U'0' + ( v % 10 );
    }
    while ( v /= 10 );

    std::reverse( begin, it );
    return it;
}

[[maybe_unused]]
inline char32_t* toChars( char32_t* begin, char32_t* end, float f )
{
    int32_t a = static_cast<int32_t>( f );
    static constexpr float maxDecimalPlaces = 10000.0f; // pow( 10.0f, number of digits )
    uint32_t b = static_cast<uint32_t>( maxDecimalPlaces * ( f - static_cast<float>( a ) ) );
    char32_t* it = toChars<int32_t>( begin, end, a );
    *it++ = U'.';
    return toChars<uint32_t>( it, end, b );
}

template <typename T>
requires ( std::is_integral_v<T> || std::is_same_v<T, float> )
inline std::pmr::u32string toString( T t )
{
    std::array<char32_t, 20> ret;
    char32_t* end = toChars( ret.data(), ret.data() + ret.size(), t );
    return std::pmr::u32string{ ret.data(), end };
}

template <typename T>
std::pmr::u32string intToUTF32( T t )
{
    return toString( t );
}

bool intersectLineSphere( const math::vec3& p1, const math::vec3& p2, const math::vec3& ps, float radius ) noexcept;

math::vec3 interceptTarget( const math::vec3& dir, const math::vec3& pos, const math::vec3& tgtPos, float turnrate ) noexcept;

namespace axis {
static constexpr math::vec3 x{ 1.0f, 0.0f, 0.0f };
static constexpr math::vec3 y{ 0.0f, 1.0f, 0.0f };
static constexpr math::vec3 z{ 0.0f, 0.0f, 1.0f };
}
