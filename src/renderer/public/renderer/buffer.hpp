#pragma once

#include <cstdint>

struct Buffer {
    enum struct Lifetime {
        ePersistent, // is alive until explicitly deleted
        eOneTimeUse, // get deleted after using
    };

    enum struct Status {
        eNone,
        ePending,
        eReady,
    };

    uint64_t m_id = 0;
    Lifetime m_lifetime = Lifetime::ePersistent;
    Status m_status = Status::eNone;

    ~Buffer() noexcept = default;
    constexpr Buffer() noexcept = default;
    constexpr explicit Buffer( uint64_t id, Lifetime lft ) noexcept
        : m_id{ id }, m_lifetime{ lft } { }
    constexpr explicit Buffer( uint64_t id, Lifetime lft, Status s ) noexcept
        : m_id{ id }, m_lifetime{ lft }, m_status{ s } { }
    constexpr Buffer( const Buffer& ) noexcept = default;
    constexpr Buffer( Buffer&& ) noexcept = default;
    constexpr Buffer& operator = ( const Buffer& ) noexcept = default;
    constexpr Buffer& operator = ( Buffer&& ) noexcept = default;

    constexpr operator Status () const noexcept
    {
        return m_status;
    }

    operator bool () const = delete;
    bool operator ! () const = delete;

};
