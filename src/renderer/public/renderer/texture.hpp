#pragma once

#include <cstdint>

struct Texture {

    enum struct Format {
        eRGB,
        eRGBA,
    };

    union {
        void* ptr;
        uint32_t id;
    } data{};

    ~Texture() noexcept = default;
    constexpr explicit Texture( uint32_t id ) noexcept
    {
        data.id = id;
    }

    constexpr Texture( void* p ) noexcept
    : data{ .ptr = p }
    {}

    constexpr Texture() noexcept = default;
    constexpr Texture( const Texture& ) noexcept = default;
    constexpr Texture( Texture&& ) noexcept = default;
    constexpr Texture& operator = ( const Texture& ) noexcept = default;
    constexpr Texture& operator = ( Texture&& ) noexcept = default;

    constexpr operator bool () const noexcept
    {
        return data.ptr;
    }
};
