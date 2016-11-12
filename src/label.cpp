#include "label.hpp"

#include <cassert>

void Label::clear()
{
    m_text.clear();
}

Label::Label( const Font* f, const std::string& str ) :
    m_font( f ),
    m_text( str ),
    m_x( 0 ),
    m_y( 0 ),
    m_xOffset( 0 ),
    m_yOffset( 0 ),
    m_isVisible( true ),
    m_alignmentHorizontal( Alignment::Left ),
    m_alignmentVertical( Alignment::Bottom )
{
}

void Label::setFont( const Font* f )
{
    assert( f );
    m_font = f;
}

void Label::setText( const std::string& str )
{
    m_text = str;
    recalcOffset();
}

std::string Label::text() const
{
    return m_text;
}

void Label::draw() const
{
    if ( !m_font ) {
        return;
    }
//     assert( m_font );
    if ( !m_isVisible ) {
        return;
    }
    SHADER::pushMatrix();
        SHADER::translate( m_x - m_xOffset, m_y - m_yOffset, 0 );
        m_font->PrintTekst( 0, 0, m_text.c_str() );
    SHADER::popMatrix();
}

void Label::move( double x, double y )
{
    m_x = x;
    m_y = y;
}

void Label::recalcOffset()
{
//     assert( m_font );
    if ( !m_font ) {
        return;
    }
    assert( m_font );
    switch ( m_alignmentHorizontal ) {
        case Alignment::Left:
            m_xOffset = 0;
            break;
        case Alignment::Center:
            m_xOffset = m_font->GetTextLength( m_text ) / 2;
            break;
        case Alignment::Right:
            m_xOffset = m_font->GetTextLength( m_text );
            break;
        default:
            assert( !"unreachable" );
            break;
    }

    // TODO: calc vertical offset;
}

void Label::setAlignment( Alignment::Horizontal horizontal, Alignment::Vertical vertical )
{
    m_alignmentHorizontal = horizontal;
    m_alignmentVertical = vertical;
    recalcOffset();
}

void Label::setVisible( bool b )
{
    m_isVisible = b;
}

bool Label::isVisible() const
{
    return m_isVisible;
}
