#pragma once

#include <string_view>
#include <charconv>
#include <type_traits>

class Args
{
    int m_argc = 0;
    const char** m_argv = nullptr;

    int findVariable( std::string_view name ) const
    {
        for ( int i = 1; i < m_argc; ++i ) {
            if ( name == m_argv[ i ] ) return i;
        }
        return m_argc;
    }

public:
    Args( int argc, const char** argv ) noexcept
    : m_argc{ argc }
    , m_argv{ argv }
    {}

    operator bool () const
    {
        return m_argc > 1;
    }

    [[nodiscard]]
    bool read( std::string_view name ) const
    {
        return findVariable( name ) < m_argc;
    }

    bool read( std::string_view name, std::string_view& value ) const noexcept
    {
        int i = findVariable( name ) + 1;
        if ( i >= m_argc ) return false;
        value = m_argv[ i ];
        return true;
    }

    template <typename T>
    requires std::is_fundamental_v<T>
    bool read( std::string_view name, T& value ) const noexcept
    {
        int i = findVariable( name ) + 1;
        if ( i >= m_argc ) return false;

        T t{};
        std::string_view v = m_argv[ i ];
        auto ret = std::from_chars( v.begin(), v.end(), t );
        if ( ret.ec != std::errc{} ) {
            return false;
        }
        value = t;
        return true;
    }
};
