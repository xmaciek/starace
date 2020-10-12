#pragma once

#include <cstdint>

struct Texture {

    enum struct Format {
        eRGB,
        eRGBA,
    };

    uint64_t m_data = 0;
    ~Texture() noexcept = default;
    constexpr Texture() noexcept = default;
    constexpr Texture( const Texture& ) noexcept = default;
    constexpr Texture( Texture&& ) noexcept = default;
    constexpr Texture& operator = ( const Texture& ) noexcept = default;
    constexpr Texture& operator = ( Texture&& ) noexcept = default;
};
