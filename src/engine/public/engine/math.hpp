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

static constexpr float pi = std::numbers::pi_v<float>;

inline auto abs( const auto& t )
{
    return glm::abs( t );
};

inline auto acos( const auto& t )
{
    return glm::acos( t );
};

inline float atan( float f )
{
    return static_cast<float>( std::atan( f ) );
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

inline float fmod( float a, float b )
{
    return std::fmod( a, b );
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

inline float tan( float f )
{
    return static_cast<float>( std::tan( f ) );
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

inline float sin( float f )
{
    return std::sin( f );
}

inline auto sin( const auto& v )
{
    return glm::sin( v );
}

inline auto scale( const auto& TMat, const auto& TVec )
{
    return glm::scale( TMat, TVec );
};

inline float sqrt( float f )
{
    return std::sqrt( f );
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
