#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>
#include <memory_resource>
#include <random>
#include <string>
#include <type_traits>

using Random = std::minstd_rand;

glm::vec3 project3dTo2d( const glm::mat4& mvp, const glm::vec3& point, const glm::vec2& viewport );
bool isOnScreen( const glm::vec3& point, const glm::vec2& viewport );
bool isOnScreen( const glm::mat4& mvp, const glm::vec3& point, const glm::vec2& viewport );
float randomRange( float a, float b );

constexpr double colorHalf( double col ) noexcept
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}

constexpr static float operator ""_deg ( long double f ) noexcept
{
    return glm::radians( static_cast<float>( f ) );
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

namespace axis {
static constexpr glm::vec3 x{ 1.0f, 0.0f, 0.0f };
static constexpr glm::vec3 y{ 0.0f, 1.0f, 0.0f };
static constexpr glm::vec3 z{ 0.0f, 0.0f, 1.0f };
}
