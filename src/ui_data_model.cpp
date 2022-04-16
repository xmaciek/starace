#include "ui_data_model.hpp"

#include <cassert>

namespace ui {

void DataModel::activate( size_type )
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

}
