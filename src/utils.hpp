#pragma once

#include <engine/math.hpp>

#include <algorithm>
#include <array>
#include <charconv>
#include <memory_resource>
#include <random>
#include <string>
#include <type_traits>

using Random = std::minstd_rand;

math::vec3 project3dTo2d( const math::mat4& mvp, const math::vec3& point, const math::vec2& viewport );
bool isOnScreen( const math::vec3& point, const math::vec2& viewport );
bool isOnScreen( const math::mat4& mvp, const math::vec3& point, const math::vec2& viewport );
float randomRange( float a, float b );

template <typename T>
char32_t* toChars( char32_t* begin, char32_t* end, T t )
{
    char tmpBuff[ 20 ]{};
    const auto res = std::to_chars( std::begin( tmpBuff ), std::end( tmpBuff ), t );
    const auto rangeSize = std::min( std::distance( begin, end ), std::distance( std::begin( tmpBuff ), res.ptr ) );
    std::copy_n( std::begin( tmpBuff ), rangeSize, begin );
    return begin + rangeSize;
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
inline const math::vec3 x{ 1.0f, 0.0f, 0.0f };
inline const math::vec3 y{ 0.0f, 1.0f, 0.0f };
inline const math::vec3 z{ 0.0f, 0.0f, 1.0f };
}
