#pragma once

#include <ui/data_model.hpp>
#include <ui/property.hpp>
#include "utils.hpp"

#include <cassert>
#include <memory_resource>
#include <string>
#include <type_traits>
#include <vector>

namespace ui {

template <typename T>
concept HasU32StringCastOperator = requires ( const T& t ) { static_cast<std::pmr::u32string>( t ); };

template <typename T>
class Option : public DataModel {
public:
    using FnToString = std::function<std::pmr::u32string(const T&)>;

private:
    size_type m_revision = 0;
    size_type m_currentIndex = 0;
    std::pmr::vector<T> m_values;
    FnToString m_toString{};

public:
    std::function<void()> m_onSelect{};

    virtual ~Option() noexcept = default;
    Option() noexcept = default;

    inline Option( size_type currentIndex ) noexcept
    requires std::is_same_v<bool, T>
    : m_currentIndex{ currentIndex }
    , m_values{ false, true }
    , m_toString{ []( bool b ) { return std::pmr::u32string{ g_uiProperty.localize( b ? "on"_hash : "off"_hash ) }; } }
    {}

    inline Option( size_type currentIndex, std::pmr::vector<T>&& values ) noexcept
    requires HasU32StringCastOperator<T>
    : m_currentIndex{ currentIndex }
    , m_values( std::move( values ) )
    , m_toString( []( const T& t ) { return static_cast<std::pmr::u32string>( t ); } )
    {}

    inline Option( size_type currentIndex, std::pmr::vector<T>&& values, FnToString&& toString ) noexcept
    : m_currentIndex{ currentIndex }
    , m_values( std::move( values ) )
    , m_toString( std::move( toString ) )
    {}

    inline Option( size_type currentIndex ) noexcept
    requires ( std::is_same_v<T, std::pmr::string> )
    : m_currentIndex{ currentIndex }
    , m_toString( []( const auto& s ) { return std::pmr::u32string{ s.begin(), s.end() }; } )
    {}

    inline Option( size_type currentIndex ) noexcept
    requires HasU32StringCastOperator<T>
    : m_currentIndex{ currentIndex }
    , m_toString( []( const T& t ) { return static_cast<std::pmr::u32string>( t ); } )
    {}

    inline Option( size_type currentIndex, FnToString&& fn ) noexcept
    : m_currentIndex{ currentIndex }
    , m_toString{ std::move( fn ) }
    {}


    inline T value() const noexcept
    {
        if ( m_values.empty() ) return {};
        assert( m_currentIndex < m_values.size() );
        return m_values[ m_currentIndex ];
    }

    inline void assign( const T& t ) noexcept
    {
        auto it = std::ranges::find( m_values, t );
        if ( it == m_values.end() ) {
            it = m_values.begin();
        }
        select( (size_type)std::distance( m_values.begin(), it ) );
    }

    inline bool assignIf( auto&& cmp ) noexcept
    {
        auto it = std::ranges::find_if( m_values, std::move( cmp ) );
        if ( it == m_values.end() ) {
            return false;
        }
        select( (size_type)std::distance( m_values.begin(), it ) );
        return true;
    }

    virtual size_type current() const override
    {
        return m_currentIndex;
    }

    virtual size_type revision() const override
    {
        return m_revision;
    }

    virtual size_type size() const override
    {
        return static_cast<size_type>( m_values.size() );
    }

    virtual std::pmr::u32string at( size_type i ) const override
    {
        if ( m_values.empty() ) return {};
        assert( i < m_values.size() );
        if constexpr ( std::is_same_v<T, std::pmr::u32string> ) {
            return m_values[ i ];
        }
        else {
            if ( m_toString ) return m_toString( m_values[ i ] );
            assert( !"missing to string fn" );
            return U"<Error>";
        }
    }

    virtual void select( size_type i ) override
    {
        if ( m_values.empty() ) return;
        assert( i < m_values.size() );
        m_currentIndex = i;
        m_revision++;
        if ( m_onSelect ) m_onSelect();
    }

    virtual void refresh( size_type i = 1 ) override
    {
        m_revision += i;
    }

    void addOption( T&& t )
    {
        m_values.emplace_back( std::forward<T>( t ) );
        refresh();
    }

    void setData( size_type current, std::pmr::vector<T>&& values )
    {
        m_currentIndex = current;
        m_values = std::move( values );
        refresh();
    }
};


}
