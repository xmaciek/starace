#include "Map.h"

void Map::Draw()
{
#if 0
    glPushMatrix();
    glDisable(GL_FOG);
    glEnable(GL_TEXTURE_2D);

    //back
    glColor3f(1,1,1);
    glBindTexture(GL_TEXTURE_2D, BACK); 
    glBegin(GL_QUADS);
      glTexCoord2f(min, min);
      glVertex3d(-v1, -v1, -v1);
      glTexCoord2f(max, min);
      glVertex3d(v1, -v1, -v1);
      glTexCoord2f(max, max);
      glVertex3d(v1, v1, -v1);
      glTexCoord2f(min, max);
      glVertex3d(-v1, v1, -v1);
    glEnd();

    //front
    glBindTexture(GL_TEXTURE_2D, FRONT); 
    glBegin(GL_QUADS);
      glTexCoord2f(min, max);
      glVertex3d(-v1, v1, v1);
      glTexCoord2f(max, max);
      glVertex3d(v1, v1, v1);
      glTexCoord2f(max, min);
      glVertex3d(v1, -v1, v1);
      glTexCoord2f(min, min);
      glVertex3d(-v1, -v1, v1);
    glEnd();
      
    //left 
    glBindTexture(GL_TEXTURE_2D, LEFT); 
    glBegin(GL_QUADS);
      glTexCoord2f(min, min);
      glVertex3d(-v1, -v1, v1);
      glTexCoord2f(max, min);
      glVertex3d(-v1, -v1, -v1);
      glTexCoord2f(max, max);
      glVertex3d(-v1, v1, -v1);
      glTexCoord2f(min, max);
      glVertex3d(-v1, v1, v1);
    glEnd();
      
            
    //right
    glBindTexture(GL_TEXTURE_2D, RIGHT); 
    glBegin(GL_QUADS);
      glTexCoord2f(min, max);
      glVertex3d(v1, v1, v1);
      glTexCoord2f(max, max);
      glVertex3d(v1, v1, -v1);
      glTexCoord2f(max, min);
      glVertex3d(v1, -v1, -v1);
      glTexCoord2f(min, min);
      glVertex3d(v1, -v1, v1);
    glEnd();
      
      //top
    glBindTexture(GL_TEXTURE_2D, TOP); 
    glBegin(GL_QUADS);
      glTexCoord2f(min, min);
      glVertex3d(-v1, v1, v1);
      glTexCoord2f(max, min);
      glVertex3d(-v1, v1, -v1);
      glTexCoord2f(max, max);
      glVertex3d(v1, v1, -v1);
      glTexCoord2f(min, max);
      glVertex3d(v1, v1, v1);
    glEnd();
      
      //bottom
    glBindTexture(GL_TEXTURE_2D, BOTTOM); 
    glBegin(GL_QUADS);
      glTexCoord2f(min, min);
      glVertex3d(-v1, -v1, v1);
      glTexCoord2f(max, min);
      glVertex3d(v1, -v1, v1);
      glTexCoord2f(max, max);
      glVertex3d(v1, -v1, -v1);
      glTexCoord2f(min, max);
      glVertex3d(-v1, -v1, -v1);
      
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_FOG);
    
    glEnable(GL_BLEND);
    glColor4f(1,1,1,0.4);
    glBegin(GL_LINES);
    for (drawing_i=0; drawing_i<particle.size(); drawing_i++) {
      glVertex3d(particle.at(drawing_i).x, particle.at(drawing_i).y, particle.at(drawing_i).z);
      glVertex3d(particle.at(drawing_i).x+particleLength.x, particle.at(drawing_i).y+particleLength.y, particle.at(drawing_i).z+particleLength.z);
      
    }
    glEnd();
    glDisable(GL_BLEND);

    glPopMatrix();
  }
      
      
      
  
  void Map::Update() {    
    for (update_I=0; update_I<particle.size(); update_I++) {
      particle.at(update_I) = particle.at(update_I) + jetVelocity;
      if (distance_v(particle.at(update_I), jetPosition)>=1.5) {
        particle.at(update_I).x = random_range(jetPosition.x-1, jetPosition.x+1);
        particle.at(update_I).y = random_range(jetPosition.y-1, jetPosition.y+1);
        particle.at(update_I).z = random_range(jetPosition.z-1, jetPosition.z+1);
        
        
      }
    }
#endif
  }

Map::Map( const MapProto &data ) :
    m_coordQuad( 0 )
{
}

Map::Map( const std::string& fileName ) :
    m_settings( fileName ),
    m_coordQuad( 0 )
{
}

void Map::Update()
{

}

void Map::drawPreview()
{
    if ( !m_previewBuffer ) {
        m_previewBuffer = SHADER::getQuad( -0.5, -0.5, 0.5, 0.5 );
        m_coordQuad = SHADER::getQuadTextureCoord( 0, 0, 1, 1 );
        fprintf( stderr, "preview is: %s\n", m_settings.get( "preview" ).c_str() );
        m_previewTexture.load( m_settings.get( "preview" ) );
    }

    SHADER::pushMatrix();
        SHADER::setMaterial( m_previewTexture );
        SHADER::setTextureCoord( m_coordQuad );
        SHADER::drawBuffer( m_previewBuffer );
    SHADER::popMatrix();
}

static const double TEXCOORD = 1024.0;

void Map::draw()
{
    if ( !m_coordQuad ) {
        m_coordQuad = SHADER::getQuadTextureCoord( 0, 0, 1, 1 );
    }

    if ( !m_topBuffer ) {
        m_topBuffer = SHADER::getQuad( -TEXCOORD, TEXCOORD, -TEXCOORD, TEXCOORD );
        m_topTexture.load( m_settings.get( "top" ) );

        m_bottomBuffer = SHADER::getQuad( -TEXCOORD, TEXCOORD, -TEXCOORD, TEXCOORD );
        m_bottomTexture.load( m_settings.get( "bottom" ) );

        m_leftBuffer = SHADER::getQuad( -TEXCOORD, TEXCOORD, -TEXCOORD, TEXCOORD );
        m_leftTexture.load( m_settings.get( "left" ) );

        m_rightBuffer = SHADER::getQuad( -TEXCOORD, TEXCOORD, -TEXCOORD, TEXCOORD );
        m_rightTexture.load( m_settings.get( "right" ) );

        m_frontBuffer = SHADER::getQuad( -TEXCOORD, TEXCOORD, -TEXCOORD, TEXCOORD );
        m_frontTexture.load( m_settings.get( "front" ) );

        m_backBuffer = SHADER::getQuad( -TEXCOORD, TEXCOORD, -TEXCOORD, TEXCOORD );
        m_backTexture.load( m_settings.get( "back" ) );
    }

    SHADER::pushMatrix();
        SHADER::setTextureCoord( m_coordQuad );

        SHADER::pushMatrix();
            SHADER::rotate( 270, Axis::X );
            SHADER::translate( 0, TEXCOORD, 0 );
            SHADER::setMaterial( m_topTexture );
            SHADER::drawBuffer( m_topBuffer );
        SHADER::popMatrix();

        SHADER::pushMatrix();
            SHADER::rotate( 90, Axis::X );
            SHADER::translate( 0, -TEXCOORD, 0 );
            SHADER::setMaterial( m_bottomTexture );
            SHADER::drawBuffer( m_bottomBuffer );
        SHADER::popMatrix();

        SHADER::pushMatrix();
            SHADER::rotate( 90, Axis::Y );
            SHADER::translate( -TEXCOORD, 0, 0 );
            SHADER::setMaterial( m_leftTexture );
            SHADER::drawBuffer( m_leftBuffer );
        SHADER::popMatrix();

        SHADER::pushMatrix();
            SHADER::rotate( 270, Axis::Y );
            SHADER::translate( TEXCOORD, 0, 0 );
            SHADER::setMaterial( m_rightTexture );
            SHADER::drawBuffer( m_rightBuffer );
        SHADER::popMatrix();

        SHADER::pushMatrix();
            SHADER::rotate( 180, Axis::Y );
            SHADER::translate( 0, 0, TEXCOORD );
            SHADER::setMaterial( m_frontTexture );
            SHADER::drawBuffer( m_frontBuffer );
        SHADER::popMatrix();

        SHADER::pushMatrix();
            SHADER::translate( 0, 0, -TEXCOORD );
            SHADER::setMaterial( m_leftTexture );
            SHADER::drawBuffer( m_leftBuffer );
        SHADER::popMatrix();
    SHADER::popMatrix();
}

void Map::GetJetData( const Vertex &Position, const Vertex &Velocity )
{

}

void Map::releaseResources()
{
    
}

uint32_t Map::numOfEnemies() const
{
    return std::stoi( m_settings.get( "enemies" ) );
}
