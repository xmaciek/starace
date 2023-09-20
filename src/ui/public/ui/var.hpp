#pragma once

#include <ui/data_model.hpp>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>

namespace ui {

template <typename T>
class Var : public DataModel {
    T m_value{};
    std::atomic<size_type> m_current{};

public:
    virtual ~Var() noexcept override = default;

    Var() noexcept = default;
    Var( T&& value ) noexcept
    : m_value{ std::forward<T&&>( value ) }
    {
    }

    Var( const Var& ) = delete;
    Var& operator = ( const Var& ) = delete;
    Var( Var&& ) = delete;
    Var& operator = ( Var&& ) = delete;

    operator T () const noexcept { return m_value; }

    Var& operator = ( T value ) noexcept
    {
        if ( value == m_value ) {
            return *this;
        }
        m_value = std::forward<T>( value );
        m_current.fetch_add( 1 );
        return *this;
    }

    virtual size_type current() const override
    {
        return m_current.load();
    }

    virtual std::pmr::u32string at( size_type ) const override
    {
        if constexpr ( std::is_same_v<T, std::pmr::u32string> ) {
            return m_value;
        }
        else {
            char tmp[ 21 ]{};
            auto str = std::to_chars( std::begin( tmp ), std::end( tmp ), m_value );
            return std::pmr::u32string{ std::begin( tmp ), str.ptr };
        }
    }

    virtual float atF( size_type ) const override
    {
        if constexpr ( std::is_same_v<T, float> ) {
            return m_value;
        }
        else return {};
    }

};

}
