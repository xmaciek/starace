#pragma once

#include <cassert>
#include <cstdint>
#include <limits>

namespace ui {

template <typename T = uint16_t, T TInvalid = std::numeric_limits<T>::max()>
class TabOrder {
    T m_min = 0;
    T m_max = 0;
    T m_value = 0;

public:
    using value_type = T;

    TabOrder() noexcept = default;
    TabOrder( const TabOrder& ) noexcept = default;
    TabOrder& operator = ( const TabOrder& ) noexcept = default;

    TabOrder( T tvalue, T tmin, T tmax ) noexcept
    : m_min{ tmin }
    , m_max{ tmax }
    , m_value{ tvalue }
    {
        assert( m_min > TInvalid || m_max <= TInvalid );
        assert( m_value >= m_min );
        assert( m_value < m_max );
    }

    TabOrder( T tmin, T tmax ) noexcept
    : m_min{ tmin }
    , m_max{ tmax }
    , m_value{ TInvalid }
    {
        assert( m_min > TInvalid || m_max <= TInvalid );
    }

    TabOrder& operator = ( T t ) noexcept
    {
        assert( t >= m_min );
        assert( t < m_max );
        m_value = t;
        return *this;
    }

    TabOrder& operator ++ () noexcept
    {
        m_value = ( m_value == TInvalid )
            ? m_min
            : ( m_value + 1 == m_max )
                ? m_min
                : m_value + 1;
        return *this;
    }

    TabOrder& operator -- () noexcept
    {
        m_value = ( m_value == TInvalid )
            ? m_max - 1
            : ( m_value == m_min )
                ? m_max - 1
                : m_value - 1;
        return *this;
    }

    TabOrder operator ++ ( int ) noexcept
    {
        TabOrder r = *this;
        ++*this;
        return r;
    }

    TabOrder operator -- ( int ) noexcept
    {
        TabOrder r = *this;
        --*this;
        return r;
    }

    T operator * () const noexcept
    {
        return m_value;
    }

    operator bool () const noexcept
    {
        return m_value != TInvalid;
    }

    void invalidate() noexcept
    {
        m_value = TInvalid;
    }

};

}
