#pragma once

#include "engine/math.hpp"

#include <array>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>

template <typename TScalar>
class CircleGen {
    size_t m_i = 0;
    size_t m_max = 0;
    float m_angle = 0.0f;
    float m_size = 0.0f;

public:
    constexpr CircleGen( size_t segments, float size ) noexcept
    : m_max{ segments }
    , m_size{ size }
    {
        m_angle = 2.0f * math::pi / (float)( segments - 1 );
    }

    TScalar operator ()() noexcept
    {
        const float angle = m_angle * static_cast<float>( m_i++ % m_max );
        const float x = -math::cos( angle ) * m_size;
        const float y = math::sin( angle ) * m_size;
        constexpr size_t nfloat = sizeof( TScalar ) / sizeof( float );
        if constexpr ( nfloat == 2 ) {
            return { x, y };
        } else if constexpr ( nfloat == 3 ) {
            return { x, y, 0.0f };
        } else if constexpr ( nfloat == 4 ) {
            return { x, y, 0.0f, 0.0f };
        } else {
            assert( !"not supported scalar type" );
            return {};
        };
    }

    template<size_t TSegments>
    static std::array<TScalar, TSegments> getCircle( float size ) noexcept
    {
        std::array<TScalar, TSegments> ret{};
        std::generate( ret.begin(), ret.end(), CircleGen<TScalar>{ TSegments, size } );
        return ret;
    }
};

