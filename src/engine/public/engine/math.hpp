#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>

namespace math {

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using quat = glm::quat;

static constexpr auto abs = []( const auto& t )
{
    return glm::abs( t );
};

static constexpr auto acos = []( const auto& t )
{
    return glm::acos( t );
};

static constexpr auto clamp = []( const auto& Tv, const auto& TMin, const auto& TMax )
{
    return glm::clamp( Tv, TMin, TMax );
};

static constexpr auto cross = []( const auto& a, const auto& b )
{
    return glm::cross( a, b );
};

static constexpr auto distance = []( const auto& a, const auto& b )
{
    return glm::distance( a, b );
};

static constexpr auto dot = []( const auto& a, const auto& b )
{
    return glm::dot( a, b );
};

static constexpr auto inverse = []( const auto& t )
{
    return glm::inverse( t );
};

static constexpr auto length = []( const auto& t )
{
    return glm::length( t );
};

static constexpr auto normalize = []( const auto& t )
{
    return glm::normalize( t );
};

static constexpr auto ortho = []( float a, float b, float c, float d, float e, float f )
{
    return glm::ortho( a, b, c, d, e, f );
};

static constexpr auto perspective = []( float a, float b, float c, float d )
{
    return glm::perspective( a, b, c, d );
};

static constexpr auto quatLookAt = []( const auto& TQuat, const math::vec3& TVec )
{
    return glm::quatLookAt( TQuat, TVec );
};

static constexpr auto radians = []( auto t )
{
    return glm::radians( t );
};

static constexpr auto rotate( const auto& TMat, const auto& TVec )
{
    return glm::rotate( TMat, TVec );
}

static constexpr auto rotate( const auto& TMat, float f, const auto& TVec )
{
    return glm::rotate( TMat, f, TVec );
}

static constexpr auto toMat4 = []( const auto& t )
{
    return glm::toMat4( t );
};

static constexpr auto translate = []( const auto& TMat, const auto& TVec )
{
    return glm::translate( TMat, TVec );
};

static constexpr auto scale = []( const auto& TMat, const auto& TVec )
{
    return glm::scale( TMat, TVec );
};

}
