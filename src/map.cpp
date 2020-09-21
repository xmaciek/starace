#include "map.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Map::draw()
{
    glPushMatrix();
    glDisable( GL_FOG );
    glEnable( GL_TEXTURE_2D );

    //back
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, m_back );
    glBegin( GL_QUADS );
    glTexCoord2f( m_min, m_min );
    glVertex3d( -m_v1, -m_v1, -m_v1 );
    glTexCoord2f( m_max, m_min );
    glVertex3d( m_v1, -m_v1, -m_v1 );
    glTexCoord2f( m_max, m_max );
    glVertex3d( m_v1, m_v1, -m_v1 );
    glTexCoord2f( m_min, m_max );
    glVertex3d( -m_v1, m_v1, -m_v1 );
    glEnd();

    //front
    glBindTexture( GL_TEXTURE_2D, m_front );
    glBegin( GL_QUADS );
    glTexCoord2f( m_min, m_max );
    glVertex3d( -m_v1, m_v1, m_v1 );
    glTexCoord2f( m_max, m_max );
    glVertex3d( m_v1, m_v1, m_v1 );
    glTexCoord2f( m_max, m_min );
    glVertex3d( m_v1, -m_v1, m_v1 );
    glTexCoord2f( m_min, m_min );
    glVertex3d( -m_v1, -m_v1, m_v1 );
    glEnd();

    //left
    glBindTexture( GL_TEXTURE_2D, m_left );
    glBegin( GL_QUADS );
    glTexCoord2f( m_min, m_min );
    glVertex3d( -m_v1, -m_v1, m_v1 );
    glTexCoord2f( m_max, m_min );
    glVertex3d( -m_v1, -m_v1, -m_v1 );
    glTexCoord2f( m_max, m_max );
    glVertex3d( -m_v1, m_v1, -m_v1 );
    glTexCoord2f( m_min, m_max );
    glVertex3d( -m_v1, m_v1, m_v1 );
    glEnd();

    //right
    glBindTexture( GL_TEXTURE_2D, m_right );
    glBegin( GL_QUADS );
    glTexCoord2f( m_min, m_max );
    glVertex3d( m_v1, m_v1, m_v1 );
    glTexCoord2f( m_max, m_max );
    glVertex3d( m_v1, m_v1, -m_v1 );
    glTexCoord2f( m_max, m_min );
    glVertex3d( m_v1, -m_v1, -m_v1 );
    glTexCoord2f( m_min, m_min );
    glVertex3d( m_v1, -m_v1, m_v1 );
    glEnd();

    //top
    glBindTexture( GL_TEXTURE_2D, m_top );
    glBegin( GL_QUADS );
    glTexCoord2f( m_min, m_min );
    glVertex3d( -m_v1, m_v1, m_v1 );
    glTexCoord2f( m_max, m_min );
    glVertex3d( -m_v1, m_v1, -m_v1 );
    glTexCoord2f( m_max, m_max );
    glVertex3d( m_v1, m_v1, -m_v1 );
    glTexCoord2f( m_min, m_max );
    glVertex3d( m_v1, m_v1, m_v1 );
    glEnd();

    //bottom
    glBindTexture( GL_TEXTURE_2D, m_bottom );
    glBegin( GL_QUADS );
    glTexCoord2f( m_min, m_min );
    glVertex3d( -m_v1, -m_v1, m_v1 );
    glTexCoord2f( m_max, m_min );
    glVertex3d( m_v1, -m_v1, m_v1 );
    glTexCoord2f( m_max, m_max );
    glVertex3d( m_v1, -m_v1, -m_v1 );
    glTexCoord2f( m_min, m_max );
    glVertex3d( -m_v1, -m_v1, -m_v1 );

    glEnd();

    glDisable( GL_TEXTURE_2D );
    glEnable( GL_FOG );

    glEnable( GL_BLEND );
    glColor4f( 1, 1, 1, 0.4 );
    glBegin( GL_LINES );
    for ( const auto& it : m_particleList ) {
        const glm::vec3 offset = it + m_particleLength;
        glVertex3fv( glm::value_ptr( it ) );
        glVertex3fv( glm::value_ptr( offset ) );
    }
    glEnd();
    glDisable( GL_BLEND );

    glPopMatrix();
}

void Map::update( const UpdateContext& updateContext )
{
    const glm::vec3 tmpVelocity = m_jetVelocity * -0.1f * updateContext.deltaTime;
    for ( auto& it : m_particleList ) {
        it += tmpVelocity;
        if ( glm::distance( it, m_jetPosition ) >= 1.5 ) {
            it.x = randomRange( m_jetPosition.x - 1, m_jetPosition.x + 1 );
            it.y = randomRange( m_jetPosition.y - 1, m_jetPosition.y + 1 );
            it.z = randomRange( m_jetPosition.z - 1, m_jetPosition.z + 1 );
        }
    }
}

Map::Map( const MapProto& data )
{
    m_top = loadTexture( data.TOP.c_str() );
    m_bottom = loadTexture( data.BOTTOM.c_str() );
    m_left = loadTexture( data.LEFT.c_str() );
    m_right = loadTexture( data.RIGHT.c_str() );
    m_front = loadTexture( data.FRONT.c_str() );
    m_back = loadTexture( data.BACK.c_str() );

    m_min = 0.00125;
    m_max = 0.99875;

    m_particleList.reserve( 100 );
    for ( int i = 0; i < 100; i++ ) {
        m_particleList.emplace_back( randomRange( -1, 1 ), randomRange( -1, 1 ), randomRange( -1, 1 ) );
    }

    m_v1 = 1000;
    m_v2 = 100;
}

void Map::setJetData( const glm::vec3& position, const glm::vec3& velocity )
{
    m_jetPosition = position;
    m_jetVelocity = velocity;
    m_particleLength = m_jetVelocity * 0.05f;
}

Map::~Map()
{
    glDeleteTextures( 1, &m_top );
    glDeleteTextures( 1, &m_bottom );
    glDeleteTextures( 1, &m_left );
    glDeleteTextures( 1, &m_right );
    glDeleteTextures( 1, &m_front );
    glDeleteTextures( 1, &m_back );
}
