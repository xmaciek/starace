#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <numbers>
#include <utility>

namespace math {

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using quat = glm::quat;

static_assert( alignof( vec4 ) == 16 );

static constexpr inline float pi = std::numbers::pi_v<float>;

using glm::abs;
using glm::acos;
using glm::atan;
using glm::clamp;
using glm::cos;
using glm::cross;
using glm::distance;
using glm::dot;
using glm::inverse;
using glm::length;
using glm::lookAt;
using glm::normalize;
using glm::ortho;
using glm::perspective;
using glm::quatLookAt;
using glm::radians;
using glm::rotate;
using glm::scale;
using glm::sin;
using glm::smoothstep;
using glm::sqrt;
using glm::tan;
using glm::toMat4;
using glm::translate;
using std::fmod;

inline float angle( const auto& a, const auto&b )
{
    return acos( dot( a, b ) );
}

inline auto billboard( const vec3& position, const vec3& cameraPosition, const vec3& cameraUp )
{
    const vec3 direction = glm::normalize( cameraPosition - position );
    const vec3 right = glm::cross( cameraUp, direction );
    const vec3 up = glm::cross( direction, right );
    return mat4{
        vec4{ right, 0 },
        vec4{ up, 0 },
        vec4{ direction, 0 },
        vec4{ position, 1 },
    };
};


inline float manhattan( vec3 a, vec3 b )
{
    const vec3 ret = abs( b - a );
    return ret.x + ret.y + ret.z;
}

inline float length( float f )
{
    return glm::abs( f );
}

template <typename T = float>
inline auto lerp( T a, T b, float n ) -> T
{
    return a + ( b - a ) * n;
}

template <typename T = float>
inline auto slerp( T a, T b, float n ) -> T
{
    return ( 1.0f - n ) * a + n * b;
}

template <typename T = float>
inline auto curve( T a, T b, T c, float n ) -> T
{
    return lerp( lerp( a, b, n ), lerp( b, c, n ), n );
}


// non-linear interpolate
template <typename T>
inline T nonlerp( const T& a, const T& b, float n )
{
    return math::lerp( a, b, smoothstep( 0.0f, 1.0f, n ) );
}

inline float normalize( float f )
{
    return f < 0.0f ? -1.0f : 1.0f;
}

inline float pointLineDistance( const math::vec3& point, const math::vec3& lineDir, const math::vec3& lineOrg ) noexcept
{
    glm::vec3 p = glm::closestPointOnLine( point, lineOrg, lineOrg + lineDir * 1000.0f );
    return distance( point, p );
}

template <uint32_t TRow, uint32_t TCol>
inline vec4 makeUVxywh( uint32_t x, uint32_t y ) noexcept
{
    const float w = 1.0f / static_cast<float>( TRow );
    const float h = 1.0f / static_cast<float>( TCol );
    return vec4{ w * static_cast<float>( x ), h * static_cast<float>( y ), w, h };
}

}
