#pragma once

#include "ui_data_model.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace ui {

template <typename T>
class Var : public DataModel {
    T m_value{};
    std::atomic<size_type> m_current{};

public:
    ~Var() noexcept
    {
        auto it = std::find_if( g_gameUiDataModels.begin(), g_gameUiDataModels.end(),
            [ptr = this]( const auto& pair ) { return pair.second == ptr; } );
        if ( it != g_gameUiDataModels.end() ) {
            g_gameUiDataModels.erase( it );
        }
    }

    Var() noexcept = default;
    Var( std::string_view symbol, T&& value ) noexcept
    : m_value{ std::forward<T&&>( value ) }
    {
        g_gameUiDataModels.emplace( std::make_pair( symbol, this ) );
    }

    Var( const Var& ) = delete;
    Var& operator = ( const Var& ) = delete;
    Var( Var&& ) = delete;
    Var& operator = ( Var&& ) = delete;

    operator T () const noexcept { return m_value; }

    Var& operator = ( T value ) noexcept
    {
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
        auto str = std::to_string( m_value );
        return std::pmr::u32string{ str.begin(), str.end() };
    }

};

template <>
inline std::pmr::u32string Var<std::pmr::u32string>::at( size_type ) const
{
    return m_value;
}

}
