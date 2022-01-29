#include "shield.hpp"

Shield::Shield( double radiusA, double )
: m_radius( static_cast<float>( radiusA ) )
{
}

void Shield::update( const UpdateContext& updateContext )
{
    m_rotAngle += 60.0f * updateContext.deltaTime;
    if ( m_rotAngle >= 360 ) {
        m_rotAngle -= 360;
    }
}

void Shield::render( RenderContext ) const
{

}

void Shield::setColor( const math::vec4& c )
{
    m_color = c;
}
