#pragma once

#include <engine/math.hpp>

#include <algorithm>
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

// TODO: copy-less conversion
template <typename T>
std::pmr::u32string intToUTF32( T t )
{
    static_assert( std::is_integral_v<T> );
    const std::string str = std::to_string( t );
    std::pmr::u32string ret;
    ret.resize( str.size() );
    std::copy( str.begin(), str.end(), ret.begin() );
    return ret;
}

bool intersectLineSphere( const math::vec3& p1, const math::vec3& p2, const math::vec3& ps, float radius ) noexcept;

namespace axis {
static constexpr math::vec3 x{ 1.0f, 0.0f, 0.0f };
static constexpr math::vec3 y{ 0.0f, 1.0f, 0.0f };
static constexpr math::vec3 z{ 0.0f, 0.0f, 1.0f };
}
