#include "Thruster.h"
#include "shader.hpp"

#include <cassert>
#include <cstring>

Thruster::Thruster( double length, double radiust ) :
    m_inner( 0 ),
    m_outter( 0 ),
    m_innerColor( 0 ),
    m_outterColor( 0 ),
    m_scaleSwitcher( 0 ),
    m_scale( 1.0 ),
    m_length( length ),
    m_radiust( radiust ),
    m_innerCircle( 32, radiust * 0.6 ),
    m_outterCircle( 32, radiust )
{
}


void Thruster::SetLength( double newLength )
{
    m_length = newLength;
}

void Thruster::SetColor( uint64_t num, float* colorData ) {
    assert( num <= 3 );
    memcpy( m_color[num], colorData, sizeof(float) * 4 );
}

void Thruster::Update() {
    m_scale = ( m_scaleSwitcher < 3 ) ? m_length : m_length * 0.95;
    m_scaleSwitcher = ( m_scaleSwitcher + 1 ) % 6;
}

static uint64_t colorArrayToBuffer( uint64_t segments, float* color1, float* color2 )
{
    assert( color1 );
    assert( color2 );

    std::vector<double> array;
    array.push_back( color1[0] );
    array.push_back( color1[1] );
    array.push_back( color1[2] );
    array.push_back( color1[3] );
    for ( uint64_t i=0; i<segments; i++ ) {
        array.push_back( color2[0] );
        array.push_back( color2[1] );
        array.push_back( color2[2] );
        array.push_back( color2[3] );
    }
    return SHADER::makeBuffer( array );
}

static uint64_t coneForCircle( const Circle &circle )
{
    std::vector<double> array;
    array.push_back( 0 );
    array.push_back( 0 );
    array.push_back( 1 );
    for ( uint64_t i=0; i<circle.segments(); i++ ) {
        array.push_back( circle.x( i ) );
        array.push_back( circle.y( i ) );
        array.push_back( 0 );
    }
    array.push_back( circle.x( 0 ) );
    array.push_back( circle.y( 0 ) );
    array.push_back( 0 );
    return SHADER::makeBuffer( array );
}

void Thruster::DrawAt( double x, double y, double z ) {
    if ( !m_innerColor ) { m_innerColor = colorArrayToBuffer( m_innerCircle.segments() + 2, m_color[0], m_color[1] ); }
    if ( !m_outterColor ) { m_outterColor = colorArrayToBuffer( m_outterCircle.segments() + 2, m_color[2], m_color[3] ); }
    if ( !m_inner ) { m_inner = coneForCircle( m_innerCircle ); }
    if ( !m_outter ) { m_outter = coneForCircle( m_outterCircle ); }
    SHADER::pushMatrix();
        SHADER::translate( x, y, z );
        SHADER::scale( 1.0, 1.0, m_scale );
        SHADER::setColorArray( m_innerColor );
        SHADER::draw( GL_TRIANGLE_FAN, m_inner, m_innerCircle.segments() + 2 );
        SHADER::setColorArray( m_outterColor );
        SHADER::draw( GL_TRIANGLE_FAN, m_outter, m_outterCircle.segments() + 2 );
    SHADER::popMatrix();
}
