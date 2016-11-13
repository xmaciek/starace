#include "valuebar.hpp"

#include "shader.hpp"

ValueBar::ValueBar() :
    m_value( 0.0 )
{
    resize( 36, 96 );
}

void ValueBar::setText( const std::string& text, const Font* font )
{
    m_text.setFont( font );
    m_text.setText( text );
    m_text.setAlignment( Alignment::Center, Alignment::Bottom );
    m_text.move( width() / 2, 98 );
}

void ValueBar::setValue( double value )
{
    m_value = value;
}

static Buffer getOutlineBuffer( double w, double h )
{
    std::vector<double> arr;
    arr.push_back( 0 );
    arr.push_back( h );
    arr.push_back( 0 );

    arr.push_back( 0 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( w );
    arr.push_back( 0 );
    arr.push_back( 0 );

    return SHADER::makeBuffer( arr, Buffer::LineStrip );
}

static double redChannel( double c )
{
    return std::min( ( 1.0 - c ) / 0.666,  1.0 );
}

static double greenChannel( double c )
{
    return std::min( c / 0.666, 1.0 );
}

void ValueBar::draw() const
{
    if ( !m_isVisible ) {
        return;
    }
    if ( !m_outline ) {
        m_outline = getOutlineBuffer( width(), height() );
        m_bar = SHADER::getQuad( 0, 0, width() - 8, height() - 8 );
    }

    SHADER::pushMatrix();
        moveToPosition();
        SHADER::pushMatrix();
            SHADER::drawBuffer( m_outline );
            SHADER::translate( 4, 4, 0 );
            SHADER::scale( 1.0, m_value, 1.0 );
            SHADER::setColor( redChannel( m_value ), greenChannel( m_value ), 0.0, 1.0 );
            SHADER::drawBuffer( m_bar );
        SHADER::popMatrix();
        m_text.draw();
    SHADER::popMatrix();
}
