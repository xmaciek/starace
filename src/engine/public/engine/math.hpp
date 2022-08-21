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

namespace math {

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using quat = glm::quat;

static constexpr float pi = std::numbers::pi_v<float>;

inline auto abs( const auto& t )
{
    return glm::abs( t );
};

inline auto acos( const auto& t )
{
    return glm::acos( t );
};

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

inline auto clamp( const auto& Tv, const auto& TMin, const auto& TMax )
{
    return glm::clamp( Tv, TMin, TMax );
};

inline float cos( float f )
{
    return glm::cos( f );
}

inline auto cross( const auto& a, const auto& b )
{
    return glm::cross( a, b );
};

inline float distance( const auto& a, const auto& b )
{
    return glm::distance( a, b );
};

inline float dot( const auto& a, const auto& b )
{
    return glm::dot( a, b );
};

inline float angle( const auto& a, const auto&b )
{
    return acos( dot( a, b ) );
}

inline auto inverse( const auto& t )
{
    return glm::inverse( t );
};

inline float length( float f )
{
    return glm::abs( f );
}

inline auto length( const auto& t )
{
    return glm::length( t );
};

template <typename T = float>
inline auto lerp( const T& a, const T&b, float n ) -> T
{
    return std::lerp( a, b, n );
}

template <>
inline auto lerp<math::vec4>( const math::vec4& a, const math::vec4& b, float n ) -> math::vec4
{
    return math::vec4{
        lerp( a.x, b.x, n ),
        lerp( a.y, b.y, n ),
        lerp( a.z, b.z, n ),
        lerp( a.w, b.w, n )
    };
}

inline auto lookAt( const auto& cameraPosition, const auto& target, const auto& cameraUp )
{
   return glm::lookAt( cameraPosition, target, cameraUp );
}

// non-linear interpolate
template <typename T>
inline T nonlerp( const T& a, const T& b, float n )
{
    const float n2 = 0.5f + 0.5f * -math::cos( n * math::pi );
    return math::lerp( a, b, n2 );
}


inline float normalize( float f )
{
    return f < 0.0f ? -1.0f : 1.0f;
}

inline auto normalize( const auto& t )
{
    return glm::normalize( t );
};


inline auto ortho( float a, float b, float c, float d, float e, float f )
{
    return glm::ortho( a, b, c, d, e, f );
};

inline auto perspective( float a, float b, float c, float d )
{
    return glm::perspective( a, b, c, d );
};

inline auto quatLookAt( const auto& TQuat, const math::vec3& TVec )
{
    return glm::quatLookAt( TQuat, TVec );
};

inline auto radians( auto t )
{
    return glm::radians( t );
};

inline auto rotate( const auto& TMat, const auto& TVec )
{
    return glm::rotate( TMat, TVec );
}

inline auto rotate( const auto& TMat, float f, const auto& TVec )
{
    return glm::rotate( TMat, f, TVec );
}

inline auto toMat4( const auto& t )
{
    return glm::toMat4( t );
};

inline auto translate( const auto& TMat, const auto& TVec )
{
    return glm::translate( TMat, TVec );
};

inline auto scale( const auto& TMat, const auto& TVec )
{
    return glm::scale( TMat, TVec );
};

inline float pointLineDistance( const math::vec3& point, const math::vec3& lineDir, const math::vec3& lineOrg ) noexcept
{
    glm::vec3 p = glm::closestPointOnLine( point, lineOrg, lineOrg + lineDir * 1000.0f );
    return distance( point, p );
}

}
