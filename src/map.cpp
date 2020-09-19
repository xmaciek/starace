#include "map.hpp"

void Map::Draw()
{
    glPushMatrix();
    glDisable( GL_FOG );
    glEnable( GL_TEXTURE_2D );

    //back
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, BACK );
    glBegin( GL_QUADS );
    glTexCoord2f( min, min );
    glVertex3d( -v1, -v1, -v1 );
    glTexCoord2f( max, min );
    glVertex3d( v1, -v1, -v1 );
    glTexCoord2f( max, max );
    glVertex3d( v1, v1, -v1 );
    glTexCoord2f( min, max );
    glVertex3d( -v1, v1, -v1 );
    glEnd();

    //front
    glBindTexture( GL_TEXTURE_2D, FRONT );
    glBegin( GL_QUADS );
    glTexCoord2f( min, max );
    glVertex3d( -v1, v1, v1 );
    glTexCoord2f( max, max );
    glVertex3d( v1, v1, v1 );
    glTexCoord2f( max, min );
    glVertex3d( v1, -v1, v1 );
    glTexCoord2f( min, min );
    glVertex3d( -v1, -v1, v1 );
    glEnd();

    //left
    glBindTexture( GL_TEXTURE_2D, LEFT );
    glBegin( GL_QUADS );
    glTexCoord2f( min, min );
    glVertex3d( -v1, -v1, v1 );
    glTexCoord2f( max, min );
    glVertex3d( -v1, -v1, -v1 );
    glTexCoord2f( max, max );
    glVertex3d( -v1, v1, -v1 );
    glTexCoord2f( min, max );
    glVertex3d( -v1, v1, v1 );
    glEnd();

    //right
    glBindTexture( GL_TEXTURE_2D, RIGHT );
    glBegin( GL_QUADS );
    glTexCoord2f( min, max );
    glVertex3d( v1, v1, v1 );
    glTexCoord2f( max, max );
    glVertex3d( v1, v1, -v1 );
    glTexCoord2f( max, min );
    glVertex3d( v1, -v1, -v1 );
    glTexCoord2f( min, min );
    glVertex3d( v1, -v1, v1 );
    glEnd();

    //top
    glBindTexture( GL_TEXTURE_2D, TOP );
    glBegin( GL_QUADS );
    glTexCoord2f( min, min );
    glVertex3d( -v1, v1, v1 );
    glTexCoord2f( max, min );
    glVertex3d( -v1, v1, -v1 );
    glTexCoord2f( max, max );
    glVertex3d( v1, v1, -v1 );
    glTexCoord2f( min, max );
    glVertex3d( v1, v1, v1 );
    glEnd();

    //bottom
    glBindTexture( GL_TEXTURE_2D, BOTTOM );
    glBegin( GL_QUADS );
    glTexCoord2f( min, min );
    glVertex3d( -v1, -v1, v1 );
    glTexCoord2f( max, min );
    glVertex3d( v1, -v1, v1 );
    glTexCoord2f( max, max );
    glVertex3d( v1, -v1, -v1 );
    glTexCoord2f( min, max );
    glVertex3d( -v1, -v1, -v1 );

    glEnd();

    glDisable( GL_TEXTURE_2D );
    glEnable( GL_FOG );

    glEnable( GL_BLEND );
    glColor4f( 1, 1, 1, 0.4 );
    glBegin( GL_LINES );
    for ( size_t i = 0; i < particle.size(); i++ ) {
        glVertex3d( particle.at( i ).x, particle.at( i ).y, particle.at( i ).z );
        glVertex3d( particle.at( i ).x + particleLength.x, particle.at( i ).y + particleLength.y, particle.at( i ).z + particleLength.z );
    }
    glEnd();
    glDisable( GL_BLEND );

    glPopMatrix();
}

void Map::Update()
{
    for ( size_t i = 0; i < particle.size(); i++ ) {
        particle.at( i ) = particle.at( i ) + jetVelocity;
        if ( distance_v( particle.at( i ), jetPosition ) >= 1.5 ) {
            particle.at( i ).x = random_range( jetPosition.x - 1, jetPosition.x + 1 );
            particle.at( i ).y = random_range( jetPosition.y - 1, jetPosition.y + 1 );
            particle.at( i ).z = random_range( jetPosition.z - 1, jetPosition.z + 1 );
        }
    }
}

Map::Map( const MapProto& data )
{
    //     textureID = LoadTexture(tex);

    TOP = LoadTexture( data.TOP.c_str() );
    BOTTOM = LoadTexture( data.BOTTOM.c_str() );
    LEFT = LoadTexture( data.LEFT.c_str() );
    RIGHT = LoadTexture( data.RIGHT.c_str() );
    FRONT = LoadTexture( data.FRONT.c_str() );
    BACK = LoadTexture( data.BACK.c_str() );

    min = 0.00125;
    max = 0.99875;

    particle.reserve( 100 );
    for ( int i = 0; i < 100; i++ ) {
        particle.emplace_back( random_range( -1, 1 ), random_range( -1, 1 ), random_range( -1, 1 ) );
    }

    v1 = 1000;
    v2 = 100;
}

void Map::GetJetData( const Vertex& Position, const Vertex& Velocity )
{
    jetPosition = Position;
    jetVelocity = Velocity;
    particleLength = jetVelocity * 0.05;
    jetVelocity = jetVelocity * -0.1 * DELTATIME;
}

Map::~Map()
{
    std::cout << "+-- Deleting Map\n";
    glDeleteTextures( 1, &TOP );
    glDeleteTextures( 1, &BOTTOM );
    glDeleteTextures( 1, &LEFT );
    glDeleteTextures( 1, &RIGHT );
    glDeleteTextures( 1, &FRONT );
    glDeleteTextures( 1, &BACK );
    //     glDeleteTextures(1, &textureID);
    //     glDeleteTextures(1,&skyboxBACK);
    //     glDeleteTextures(1,&skyboxFRONT);
    //     glDeleteTextures(1,&skyboxLEFT);
}
