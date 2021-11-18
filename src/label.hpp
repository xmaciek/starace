#pragma once

#include "font.hpp"
#include "widget.hpp"

#include <engine/render_context.hpp>

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include <string>
#include <string_view>
#include <type_traits>

constexpr uint32_t operator ""_bit( unsigned long long n ) noexcept
{
    return 1ull << n;
}

enum struct Align : uint32_t {
    fLeft = 0_bit,
    fCenter = 1_bit,
    fRight = 2_bit,
    fBottom = 3_bit,
    fMiddle = 4_bit,
    fTop = 5_bit,
};

constexpr Align operator | ( Align a, Align b ) noexcept
{
    using T = std::underlying_type_t<Align>;
    return static_cast<Align>( static_cast<T>( a ) | static_cast<T>( b ) );
}



class Label : public Widget {
private:
    Font* m_font = nullptr;
    mutable Font::RenderText m_renderText{};
    std::pmr::string m_text{};
    glm::vec4 m_color{};
    glm::vec2 m_positionOffset{};
    Align m_align = Align::fLeft | Align::fBottom;

    template <typename T>
    static inline std::string toString( T t ) { return std::to_string( t ); }
    static inline std::string toString( const char* t ) { return t; }

public:
    Label() = default;
    Label( std::string_view, Font*, const glm::vec2& position, const glm::vec4& color );
    Label( std::string_view, Font*, Align, const glm::vec2& position, const glm::vec4& color );
    Label( Font*, Align, const glm::vec2& position, const glm::vec4& color );

    virtual void render( RenderContext ) const override;
    void setText( std::string_view );

    template <typename T>
    void arg( const T& t )
    {
        setText( toString( t ) );
    }
};
