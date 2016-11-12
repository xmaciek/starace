#include "fanrotor.hpp"

#include <vector>

#include "Circle.h"
#include "shader.hpp"

FanRotor::FanRotor() :
    m_rotation( 0.0 )
{
    m_width = 64;
    m_height = 64;
    setAlignment( Alignment::Left, Alignment::Bottom );
}

void FanRotor::setRotation( double rotation )
{
    m_rotation += rotation;
    while ( m_rotation >= 360.0 ) {
        m_rotation -= 360.0;
    }
    while ( m_rotation < 0 ) {
        m_rotation += 360.0;
    }
}

static Buffer getFanRotorRing( const Circle& circle )
{
    std::vector<double> arr;
    for ( uint32_t i=0; i<circle.segments(); i++ ) {
        arr.push_back( circle.x( i ) );
        arr.push_back( circle.y( i ) );
        arr.push_back( 0 );
    }
    return SHADER::makeBuffer( arr, Buffer::LineLoop );
}

static Buffer getFanRotorPetals( double radiust )
{
    std::vector<double> arr;

    arr.push_back( 0 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( radiust * 0.1125 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( radiust * 0.4528 );
    arr.push_back( radiust * 0.9 );
    arr.push_back( 0 );

    arr.push_back( 0 );
    arr.push_back( radiust );
    arr.push_back( 0 );

    arr.push_back( -radiust * 0.4528 );
    arr.push_back( radiust * 0.9 );
    arr.push_back( 0 );

    arr.push_back( -radiust * 0.1125 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( -radiust * 0.4528 );
    arr.push_back( -radiust * 0.9 );
    arr.push_back( 0 );

    arr.push_back( 0 );
    arr.push_back( -radiust );
    arr.push_back( 0 );

    arr.push_back( radiust * 0.4528 );
    arr.push_back( -radiust * 0.9 );
    arr.push_back( 0 );

    arr.push_back( radiust * 0.1125 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    return SHADER::makeBuffer( arr, Buffer::TriangleFan );
}

void FanRotor::draw() const
{
    if ( !m_isVisible ) {
        return;
    }

    if ( !m_fanCircle ) {
        Circle circle( 32, width() / 2 );
        m_fanCircle = getFanRotorRing( circle );
        m_fanPetals = getFanRotorPetals( circle.radiust() );
    }

    SHADER::pushMatrix();
        moveToPosition();
        SHADER::drawBuffer( m_fanCircle );
        SHADER::rotate( m_rotation, Axis::Z );
        SHADER::drawBuffer( m_fanPetals );
    SHADER::popMatrix();
}
