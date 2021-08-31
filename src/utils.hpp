#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

glm::vec3 project3dTo2d( const glm::mat4& mvp, const glm::vec3& point, const glm::vec2& viewport );
bool isOnScreen( const glm::vec3& point, const glm::vec2& viewport );
bool isOnScreen( const glm::mat4& mvp, const glm::vec3& point, const glm::vec2& viewport );
float randomRange( float a, float b );

constexpr double colorHalf( double col ) noexcept
{
    return ( col >= 0.5 ) ? 1.0 - col : col;
}

constexpr float lerp( float a, float b, float n ) noexcept
{
    const float range = b - a;
    return a + range * n;
}

constexpr static float operator ""_deg ( long double f ) noexcept
{
    return glm::radians( static_cast<float>( f ) );
}
