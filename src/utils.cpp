#include "utils.hpp"

#include <random>

float randomRange( float a, float b )
{
    static constexpr float max = (float)Random::max();
    thread_local Random random( std::random_device{}() );
    const uint64_t r = random();
    return ( b - a ) * static_cast<float>( r ) / max + a;
}

math::vec3 project3dTo2d( const math::mat4& mvp, const math::vec3& point, const math::vec2& viewport )
{
    const math::vec4 vec = mvp * math::vec4{ point, 1.0f };
    return {
        ( vec.x / vec.w + 1 ) * viewport.x * 0.5f,
        ( 1.0f - vec.y / vec.w ) * viewport.y * 0.5f,
        vec.z
    };
}

bool isOnScreen( const math::vec3& point, const math::vec2& viewport )
{
    return point.z > 0.0f
        && point.x >= 0.0f
        && point.x < viewport.x
        && point.y >= 0.0f
        && point.y < viewport.y
    ;
}

bool isOnScreen( const math::mat4& mvp, const math::vec3& point, const math::vec2& viewport )
{
    const math::vec3 vec = project3dTo2d( mvp, point, viewport );
    return isOnScreen( vec, viewport );
}

bool intersectLineSphere( const math::vec3& p1, const math::vec3& p2, const math::vec3& ps, float radius ) noexcept
{
    const math::vec3 dir = ps - p1;
    const math::vec3 ray = p2 - p1;
    const float tmp = math::dot( dir, ray );

    if ( tmp <= 0 ) {
        return math::length( dir ) < radius;
    }

    const float tmp2 = math::dot( ray, ray );
    if ( tmp2 <= tmp ) {
        return math::length( dir ) < radius;
    }

    const math::vec3 pb = p1 + ( ray * ( tmp / tmp2 ) );
    return math::length( ps - pb ) < radius;
}
