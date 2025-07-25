#include <ui/data_model.hpp>

#include <cassert>

namespace ui {

DataModel::size_type DataModel::revision() const
{
    return {};
}

DataModel::size_type DataModel::current() const
{
    return {};
}

DataModel::size_type DataModel::size() const
{
    return {};
}

std::pmr::u32string DataModel::at( size_type ) const
{
    return {};
}

float DataModel::atF( size_type ) const
{
    return {};
}

Sprite DataModel::texture( size_type ) const
{
    return {};
}

void DataModel::activate( size_type )
{
}

void DataModel::select( size_type )
{
}

void DataModel::refresh( size_type )
{
}

GenericDataModel::size_type GenericDataModel::size() const
{
    assert( m_size );
    return m_size();
}

std::pmr::u32string GenericDataModel::at( size_type i ) const
{
    assert( m_at );
    return m_at( i );
}

void GenericDataModel::activate( size_type i )
{
    if ( m_activate ) {
        m_activate( i );
    }
}

void GenericDataModel::select( size_type i )
{
    if ( m_select ) {
        m_select( i );
    }
}

void GenericDataModel::refresh( size_type i )
{
    if ( m_refresh ) {
        m_refresh( i );
    }
}

Sprite GenericDataModel::texture( size_type i ) const
{
    return m_texture ? m_texture( i ) : Sprite{};
}

GenericDataModel::size_type GenericDataModel::current() const
{
    return m_current ? m_current() : 0;
}

GenericDataModel::size_type GenericDataModel::revision() const
{
    return m_revision ? m_revision() : 0;
}

}
