#pragma once

#include <ui/data_model.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <type_traits>

namespace ui {

template <typename T>
class Var : public DataModel {
    T m_value{};
    size_type m_current{};
    size_type m_revision = 0xFFFF;

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
        m_value = std::forward<T>( value );
        m_current++;
        m_revision++;
        return *this;
    }

    virtual size_type current() const override
    {
        return m_current;
    }

    virtual size_type revision() const override
    {
        return m_revision;
    }

    virtual void refresh( size_type i = 1 ) override
    {
        m_revision += i;
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

    virtual Sprite texture( size_type ) const override
    {
        if constexpr ( std::is_same_v<T, Sprite> ) {
            return m_value;
        }
        else return {};
    }

};

}
