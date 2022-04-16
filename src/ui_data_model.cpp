#include "ui_data_model.hpp"

#include <cassert>

namespace ui {

void DataModel::activate( size_type )
{
}

void DataModel::select( size_type )
{
}

StringListModel::size_type StringListModel::size() const
{
    return static_cast<size_type>( m_data.size() );
}

std::pmr::u32string StringListModel::at( size_type i ) const
{
    assert( i < size() );
    return m_data[ i ];
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


}
