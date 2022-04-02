#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace cfg {

static constexpr std::array c_separators = {
    ' ',
    '=',
    '\n',
    '\r',
    '\t',
    '{',
    '}',
};

struct Token {
    uint32_t userEnum = 0;
    uint32_t length = 0;
    const char* data = nullptr;

    inline
    operator bool () const
    {
        return length != 0;
    }
    inline
    std::string_view operator * () const
    {
        return { data, data + length };
    }
};

class TokenIterator {

    std::span<const char> m_tokens{};
    const char* m_begin = nullptr;
    const char* m_end = nullptr;
    Token m_current{};

public:
    static constexpr uint32_t c_unknown = 0xFFFF'FFFF;
    static constexpr Token c_invalid{};

    inline
    TokenIterator( std::span<const char> t, std::span<const char> d )
    : m_tokens{ t }
    , m_begin{ &*d.begin() }
    , m_end{ &*d.end() }
    {
        ++*this;
    }

    inline
    TokenIterator& operator ++ ();
    inline
    Token operator * () const;

};


Token TokenIterator::operator * () const
{
    return m_current;
}

TokenIterator& TokenIterator::operator ++ ()
{
    if ( m_begin == m_end ) {
        m_current = c_invalid;
        return *this;
    }

    auto it = std::find_first_of( m_begin, m_end, m_tokens.begin(), m_tokens.end() );

    if ( it == m_begin ) {
        m_current = {
            .userEnum = static_cast<uint32_t>( *it ),
            .length = 1u, // TODO separator length
            .data = it,
        };
        m_begin += m_current.length;
        return *this;
    }

    m_current = {
        .userEnum = c_unknown,
        .length = static_cast<uint32_t>( it - m_begin ),
        .data = m_begin,
    };
    m_begin += m_current.length;
    return *this;
}

}
