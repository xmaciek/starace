#pragma once

#include <cstdint>

struct Buffer {
    uint64_t m_id = 0;

    ~Buffer() noexcept = default;
    constexpr Buffer() noexcept = default;
    constexpr explicit Buffer( uint64_t id ) noexcept : m_id{ id } { }
    constexpr Buffer( const Buffer& ) noexcept = default;
    constexpr Buffer( Buffer&& ) noexcept = default;
    constexpr Buffer& operator = ( const Buffer& ) noexcept = default;
    constexpr Buffer& operator = ( Buffer&& ) noexcept = default;

    constexpr operator bool () const noexcept
    {
        return m_id;
    }

    constexpr bool operator ! () const noexcept
    {
        return !m_id;
    }
};
