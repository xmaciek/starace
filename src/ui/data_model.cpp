#include <ui/data_model.hpp>

#include <cassert>

namespace ui {

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

Texture DataModel::texture( size_type ) const
{
    return {};
}

void DataModel::activate( size_type )
{
}

void DataModel::select( size_type )
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

Texture GenericDataModel::texture( size_type i ) const
{
    return m_texture ? m_texture( i ) : Texture{};
}

GenericDataModel::size_type GenericDataModel::current() const
{
    return m_current ? m_current() : 0;
}

}
