#include "shield.hpp"

#include <array>
#include <algorithm>

#include <renderer/buffer.hpp>
#include <renderer/pipeline.hpp>
#include <renderer/renderer.hpp>
#include "circle.hpp"

#include <glm/gtc/matrix_transform.hpp>

static std::array<glm::vec4, 7> getCircle( float radius ) noexcept
{
    std::array<glm::vec4, 7> ret = CircleGen<glm::vec4>::getCircle<7>( radius );
    for ( auto& it : ret ) {
        it.z = 0.1f;
    }
    return ret;
}

static const std::array<glm::vec4, 7> circle = getCircle( 0.02f );

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

void Shield::setColor( const glm::vec4& c )
{
    m_color = c;
}
