#include "widget.hpp"

#include <cassert>
#include "shader.hpp"

Widget::Widget() :
    m_width( 0.0 ), m_height( 0.0 ),
    m_x( 0.0 ), m_y( 0.0 ),
    m_xOffset( 0.0 ), m_yOffset( 0.0 ),
    m_isVisible( true ),
    m_alignmentHorizontal( Alignment::Left ),
    m_alignmentVertical( Alignment::Bottom )
{
}

Widget::~Widget()
{
}

double Widget::width() const
{
    return m_width;
}

double Widget::height() const
{
    return m_height;
}

void Widget::resize( double w, double h )
{
    m_width = w;
    m_height = h;
    recalcAlignmentOffset();
}

double Widget::x() const
{
    return m_x;
}

double Widget::right() const
{
    return x() + width();
}

double Widget::y() const
{
    return m_y;
}

void Widget::move( double x, double y )
{
    m_x = x;
    m_y = y;
    recalcAlignmentOffset();
}

void Widget::setVisible( bool b )
{
    m_isVisible = b;
}

bool Widget::isVisible() const
{
    return m_isVisible;
}

void Widget::setAlignment( Alignment::Horizontal horizontal, Alignment::Vertical vertical )
{
    m_alignmentHorizontal = horizontal;
    m_alignmentVertical = vertical;
    recalcAlignmentOffset();
}

void Widget::recalcAlignmentOffset()
{
    switch ( m_alignmentHorizontal ) {
        case Alignment::Left:
            m_xOffset = 0;
            break;
        case Alignment::Center:
            m_xOffset = m_width / 2;
            break;
        case Alignment::Right:
            m_xOffset = m_width;
            break;
        default:
            assert( !"unreachable" );
            break;
    }

    switch ( m_alignmentVertical ) {
        case Alignment::Top:
            m_yOffset = m_height;
            break;
        case Alignment::Middle:
            m_yOffset = m_height / 2;
            break;
        case Alignment::Bottom:
            m_yOffset = 0;
            break;
        default:
            assert( !"unreachable" );
            break;
    }
}

void Widget::moveToPosition() const
{
    SHADER::translate( m_x - m_xOffset, m_y - m_yOffset, 0 );
}
