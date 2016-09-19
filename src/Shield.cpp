#include "Shield.h"
#include "shader.hpp"

uint64_t Shield::s_segment = 0;

Shield::Shield(GLdouble RadiustA, GLdouble RadiustB) :
    m_circle( 6, RadiustB ),
    m_radiust( RadiustA ),
    m_rotAngle( 0 )
{
}


void Shield::Update() {
  m_rotAngle += 1;
  if (m_rotAngle>=360) { m_rotAngle -= 360; }
}

static void drawSegments( uint64_t bufferId, uint64_t size )
{
    SHADER::pushMatrix();
    for ( uint64_t i=0; i<8; i++ ) {
        SHADER::draw( GL_LINE_LOOP, bufferId, size );
        SHADER::rotate( 360.0 / 8, 1, 0, 0 );
    }
    SHADER::popMatrix();
}

void Shield::Draw()
{
    if ( !s_segment ) {
        std::vector<double> arr;
        for ( uint64_t i=0; i<m_circle.GetSegments(); i++ ) {
            arr.push_back( m_circle.GetX( i ) );
            arr.push_back( m_circle.GetY( i ) );
            arr.push_back( m_radiust );
        }
        s_segment = SHADER::makeBuffer( arr );
    }
    SHADER::pushMatrix();
        SHADER::rotate( m_rotAngle, 0, 1, 0 );
        drawSegments( s_segment, m_circle.GetSegments() );

        SHADER::rotate( m_rotAngle, 0, 0, 1 );
        drawSegments( s_segment, m_circle.GetSegments() );

        SHADER::rotate( m_rotAngle, 1, 0, 0 );
        drawSegments( s_segment, m_circle.GetSegments() );
    SHADER::popMatrix();

}
