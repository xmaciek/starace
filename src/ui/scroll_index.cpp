#include <ui/scroll_index.hpp>

#include <algorithm>

namespace ui {

ScrollIndex::ScrollIndex( value_type current, value_type max ) noexcept
{
    m_maxVisible = std::min( m_maxVisible, max );
    m_maxOffset = max - m_maxVisible;
    // TODO: figure this out
    while ( this->current() != current ) {
        increase();
    }
}


void ScrollIndex::increase()
{
    if ( ( m_index + 1 ) < m_maxVisible ) {
        m_index++;
        return;
    }
    if ( ( m_offset ) < m_maxOffset ) {
        m_offset++;
    }
}

void ScrollIndex::decrease()
{
    if ( m_index > 0 ) {
        m_index--;
        return;
    }
    if ( m_offset > 0 ) {
        m_offset--;
    }
}

void ScrollIndex::scrollUp( ScrollIndex::value_type v )
{
    m_offset = std::max<ScrollIndex::value_type>( m_offset + v, m_maxOffset );
}

void ScrollIndex::scrollDown( ScrollIndex::value_type v )
{
    m_offset = ( v > m_offset ) ? 0 : ( m_offset - v );
}

ScrollIndex::value_type ScrollIndex::current() const
{
    return m_offset + m_index;
}

ScrollIndex::value_type ScrollIndex::currentVisible() const
{
    return m_index;
}

ScrollIndex::value_type ScrollIndex::offset() const
{
    return m_offset;
}

ScrollIndex::value_type ScrollIndex::maxVisible() const
{
    return m_maxVisible;
}

void ScrollIndex::selectVisible( value_type i )
{
    m_index = std::min( i, m_maxVisible );
}



}
