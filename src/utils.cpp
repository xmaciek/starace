#include "utils.hpp"

#include <glm/vec4.hpp>

#include <random>

float randomRange( float a, float b )
{
    using Rand = std::minstd_rand;
    static constexpr float max = (float)Rand::max();
    thread_local Rand random( std::random_device{}() );
    const uint64_t r = random();
    return ( b - a ) * static_cast<float>( r ) / max + a;
}

glm::vec3 project3dTo2d( const glm::mat4& mvp, const glm::vec3& point, const glm::vec2& viewport )
{
    const glm::vec4 vec = mvp * glm::vec4{ point, 1.0f };
    return {
        ( vec.x / vec.w + 1 ) * viewport.x * 0.5f,
        ( 1.0f - vec.y / vec.w ) * viewport.y * 0.5f,
        vec.z
    };
}

bool isOnScreen( const glm::vec3& point, const glm::vec2& viewport )
{
    return point.z > 0.0f
        && point.x >= 0.0f
        && point.x < viewport.x
        && point.y >= 0.0f
        && point.y < viewport.y
    ;
}

bool isOnScreen( const glm::mat4& mvp, const glm::vec3& point, const glm::vec2& viewport )
{
    const glm::vec3 vec = project3dTo2d( mvp, point, viewport );
    return isOnScreen( vec, viewport );
}
