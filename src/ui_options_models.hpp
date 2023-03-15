#pragma once

#include <ui/data_model.hpp>
#include "ui_property.hpp"
#include "utils.hpp"

#include <cassert>
#include <memory_resource>
#include <string>
#include <type_traits>
#include <vector>

namespace ui {

template <typename T>
class OptionsArrayModel : public DataModel {
    size_type m_current = 0;
    std::pmr::vector<T> m_values;

public:
    virtual ~OptionsArrayModel() noexcept = default;
    OptionsArrayModel() noexcept = default;

    OptionsArrayModel( std::pmr::vector<T>&& vec ) noexcept
    : m_values( std::move( vec ) )
    {
    }

    T value() const
    {
        assert( m_current < m_values.size() );
        return m_values[ m_current ];
    }

    virtual size_type current() const override
    {
        return m_current;
    }

    virtual size_type size() const override
    {
        return static_cast<size_type>( m_values.size() );
    }

    virtual std::pmr::u32string at( size_type i ) const override
    {
        assert( i < m_values.size() );
        return toString( m_values[ i ] );
    }

    virtual void select( size_type i ) override
    {
        assert( i < m_values.size() );
        m_current = i;
    }

};

template <typename T>
class Option : public DataModel {
public:
    using FnToString = std::function<std::pmr::u32string(const T&)>;

private:
    size_type m_currentIndex = 0;
    std::pmr::vector<T> m_values;
    std::pmr::vector<Hash::value_type> m_locValues;
    FnToString m_toString{};

public:
    virtual ~Option() noexcept = default;
    Option() noexcept = default;

    Option( size_type currentIndex ) noexcept
    requires std::is_same_v<bool, T>
    : m_currentIndex{ currentIndex }
    , m_values{ false, true }
    , m_locValues{ "off"_hash, "on"_hash }
    {}

    Option( size_type currentIndex
        , std::pmr::vector<T>&& values
        , std::pmr::vector<Hash::value_type>&& locValues
    ) noexcept
    : m_currentIndex{ currentIndex }
    , m_values( std::move( values ) )
    , m_locValues( std::move( locValues ) )
    {}

    Option( size_type currentIndex
        , std::pmr::vector<T>&& values
        , FnToString&& toString
    ) noexcept
    : m_currentIndex{ currentIndex }
    , m_values( std::move( values ) )
    , m_toString( std::move( toString ) )
    {}

    T value() const
    {
        assert( m_currentIndex < m_values.size() );
        return m_values[ m_currentIndex ];
    }

    virtual size_type current() const override
    {
        return m_currentIndex;
    }

    virtual size_type size() const override
    {
        return static_cast<size_type>( m_values.size() );
    }

    virtual std::pmr::u32string at( size_type i ) const override
    {
        assert( i < m_values.size() );
        if ( m_toString ) return m_toString( m_values[ i ] );
        assert( i < m_locValues.size() );
        return g_uiProperty.localize( m_locValues[ i ] );
    }

    virtual void select( size_type i ) override
    {
        assert( i < m_values.size() );
        m_currentIndex = i;
    }

};


}
