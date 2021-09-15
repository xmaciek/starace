#pragma once

#include <cstdint>

struct Texture {
    void* ptr = nullptr;

    enum struct Format {
        eR,
        eRGB,
        eRGBA,
        eBGRA,
        eBGR,
    };


    ~Texture() noexcept = default;
    constexpr Texture( void* p ) noexcept
    : ptr{ p }
    {}
    constexpr Texture() noexcept = default;
    constexpr Texture( const Texture& ) noexcept = default;
    constexpr Texture( Texture&& ) noexcept = default;
    constexpr Texture& operator = ( const Texture& ) noexcept = default;
    constexpr Texture& operator = ( Texture&& ) noexcept = default;

    constexpr operator bool () const noexcept
    {
        return ptr;
    }
};
