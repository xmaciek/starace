#pragma once

#include <cstdint>

struct Buffer {
    enum struct Lifetime {
        ePersistent, // is alive until explicitly deleted
        eOneTimeUse, // get deleted after using
    };

    uint64_t m_id = 0;
    Lifetime m_lifetime = Lifetime::ePersistent;

    ~Buffer() noexcept = default;
    constexpr Buffer() noexcept = default;
    constexpr explicit Buffer( uint64_t id, Lifetime lft ) noexcept
        : m_id{ id }, m_lifetime{ lft } { }
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
