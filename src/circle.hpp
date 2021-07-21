#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <cmath>
#include <glm/vec2.hpp>

template <typename TScalar>
class CircleGen {
    size_t m_i = 0;
    size_t m_max = 0;
    float m_angle = 0.0f;
    float m_size = 0.0f;

public:
    constexpr CircleGen( size_t segments, float size ) noexcept
    : m_max{ segments - 1u }
    , m_size{ size }
    {
        m_angle = 2.0f * (float)M_PI / (float)segments;
    }

    constexpr TScalar operator ()() noexcept
    {
        const float angle = m_angle * ( m_i++ % m_max );
        const float x = -std::cos( angle ) * m_size;
        const float y = std::sin( angle ) * m_size;
        constexpr size_t nfloat = sizeof( TScalar ) / sizeof( float );
        if constexpr ( nfloat == 2 ) {
            return { x, y };
        } else if constexpr ( nfloat == 3 ) {
            return { x, y, 0.0f };
        } else if constexpr ( nfloat == 4 ) {
            return { x, y, 0.0f, 0.0f };
        } else {
            return throw("");
        };
    }

    template<size_t TSegments>
    static std::array<TScalar, TSegments> getCircle( float size ) noexcept
    {
        std::array<TScalar, TSegments> ret;
        std::generate( ret.begin(), ret.end(), CircleGen<TScalar>{ TSegments, size } );
        return ret;
    }
};

