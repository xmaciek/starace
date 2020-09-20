#include "map.hpp"

void Map::Draw()
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
        glVertex3d( it.x, it.y, it.z );
        glVertex3d( it.x + m_particleLength.x, it.y + m_particleLength.y, it.z + m_particleLength.z );
    }
    glEnd();
    glDisable( GL_BLEND );

    glPopMatrix();
}

void Map::Update()
{
    for ( auto& it : m_particleList ) {
        it += m_jetVelocity;
        if ( distance_v( it, m_jetPosition ) >= 1.5 ) {
            it.x = random_range( m_jetPosition.x - 1, m_jetPosition.x + 1 );
            it.y = random_range( m_jetPosition.y - 1, m_jetPosition.y + 1 );
            it.z = random_range( m_jetPosition.z - 1, m_jetPosition.z + 1 );
        }
    }
}

Map::Map( const MapProto& data )
{
    //     textureID = LoadTexture(tex);

    m_top = LoadTexture( data.TOP.c_str() );
    m_bottom = LoadTexture( data.BOTTOM.c_str() );
    m_left = LoadTexture( data.LEFT.c_str() );
    m_right = LoadTexture( data.RIGHT.c_str() );
    m_front = LoadTexture( data.FRONT.c_str() );
    m_back = LoadTexture( data.BACK.c_str() );

    m_min = 0.00125;
    m_max = 0.99875;

    m_particleList.reserve( 100 );
    for ( int i = 0; i < 100; i++ ) {
        m_particleList.emplace_back( random_range( -1, 1 ), random_range( -1, 1 ), random_range( -1, 1 ) );
    }

    m_v1 = 1000;
    m_v2 = 100;
}

void Map::GetJetData( const Vertex& Position, const Vertex& Velocity )
{
    m_jetPosition = Position;
    m_jetVelocity = Velocity;
    m_particleLength = m_jetVelocity * 0.05;
    m_jetVelocity = m_jetVelocity * -0.1 * DELTATIME;
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
