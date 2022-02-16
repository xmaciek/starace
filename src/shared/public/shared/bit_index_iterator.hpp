#pragma once

#include <bit>
#include <cstdint>

struct BitIndexIterator
{
    using value_type = uint64_t;
    value_type m_integer = 0;

    constexpr BitIndexIterator() noexcept = default;
    constexpr BitIndexIterator( value_type t ) noexcept
    : m_integer{ t }
    {}

    constexpr BitIndexIterator& operator = ( value_type t ) noexcept
    {
        m_integer = t;
        return *this;
    }

    constexpr uint32_t operator * () const noexcept
    {
        return static_cast<uint32_t>( std::countr_zero( m_integer ) );
    }

    constexpr operator bool () const noexcept
    {
        return m_integer != 0;
    }

    constexpr BitIndexIterator& operator ++ () noexcept
    {
        const uint32_t idx = **this;
        const value_type tint = 1ull << idx;
        m_integer &= ~tint;
        return *this;
    }

    constexpr BitIndexIterator operator ++ ( int ) noexcept
    {
        BitIndexIterator ret = *this;
        ++*this;
        return ret;
    }

    constexpr uint32_t count() const noexcept
    {
        return static_cast<uint32_t>( std::popcount( m_integer ) );
    }
};
