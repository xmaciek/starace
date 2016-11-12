#include "label.hpp"

#include <cassert>

void Label::clear()
{
    m_text.clear();
}

Label::Label( const Font* f, const std::string& str ) :
    m_font( f ),
    m_text( str )
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
    recalcAlignmentOffset();
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
        moveToPosition();
        m_font->PrintTekst( 0, 0, m_text.c_str() );
    SHADER::popMatrix();
}

void Label::recalcAlignmentOffset()
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

