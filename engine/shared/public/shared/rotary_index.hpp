#pragma once

#include <cassert>

template <typename T = int, T TMin = 0, T TMaxExclusive = 3>
class RotaryIndex {
    static_assert( TMin < TMaxExclusive );
    T m_value = TMin;

public:
    RotaryIndex() noexcept = default;
    RotaryIndex( const RotaryIndex& ) noexcept = default;
    RotaryIndex& operator = ( const RotaryIndex& ) noexcept = default;

    RotaryIndex( T t ) noexcept
    : m_value{ t }
    {
        assert( t >= TMin );
        assert( t < TMaxExclusive );
    }

    RotaryIndex& operator = ( T t ) noexcept
    {
        assert( t >= TMin );
        assert( t < TMaxExclusive );
        m_value = t;
        return *this;
    }

    RotaryIndex& operator ++ () noexcept
    {
        m_value = ( m_value + 1 == TMaxExclusive ) ? TMin : m_value + 1;
        return *this;
    }

    RotaryIndex& operator -- () noexcept
    {
        m_value = ( m_value == TMin ) ? TMaxExclusive - 1 : m_value - 1;
        return *this;
    }

    RotaryIndex operator ++ ( int ) noexcept
    {
        RotaryIndex r = *this;
        ++*this;
        return r;
    }

    RotaryIndex operator -- ( int ) noexcept
    {
        RotaryIndex r = *this;
        --*this;
        return r;
    }

    T operator * () const noexcept
    {
        assert( m_value >= TMin );
        assert( m_value < TMaxExclusive );
        return m_value;
    }

};

