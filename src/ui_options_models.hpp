#pragma once

#include "ui_data_model.hpp"
#include "utils.hpp"

#include <cassert>

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

}
