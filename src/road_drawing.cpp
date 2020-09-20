#include "road.hpp"

void Road::gameScreen()
{
    render3D();
    renderHUD();
    m_framesDone++;
    m_tempFPS += ( SDL_GetTicks() - m_timeS );
    if ( m_timePassed < time( nullptr ) ) {
        m_fps = m_framesDone;
        m_calculatedFPS = 1000.0f / ( m_tempFPS / m_framesDone ); // m_framesDone);
        m_tempFPS = 0;
        m_framesDone = 0;
        m_timePassed++;
    }
}

void Road::gameScreenPaused()
{
    gameScreen();
    drawPauseText();
}

void Road::drawAxis()
{
    glPushMatrix();
    glBegin( GL_LINES );
    glColor3f( 1, 0, 0 );
    glVertex3d( -0.1, 0, 0 );
    glVertex3d( 0.1, 0, 0 );

    glColor3f( 0, 1, 0 );
    glVertex3d( 0, -0.1, 0 );
    glVertex3d( 0, 0.1, 0 );

    glColor3f( 0, 0, 1 );
    glVertex3d( 0, 0, -0.1 );
    glVertex3d( 0, 0, 0.1 );
    glEnd();
    glPopMatrix();
}

/* HUD elements */

void Road::drawLine( GLdouble x, GLdouble y )
{
    glPushMatrix();
    glBegin( GL_LINES );
    glColor4f( 1, 0.4f, 0, 0 );
    glVertex3d( x, y, -100 );
    glColor4f( 1, 0.4f, 0, 1 );
    glVertex3d( x, y, 1 );
    glEnd();
    glPopMatrix();
}

void Road::drawHudRect( GLdouble x, GLdouble y, GLdouble w, GLdouble h )
{
    glPushMatrix();
    glTranslated( x, y, 0 );
    glBegin( GL_QUADS );
    glVertex2d( 0, 0 );
    glVertex2d( 0, h );
    glVertex2d( w, h );
    glVertex2d( w, 0 );
    glEnd();
    glPopMatrix();
}

void Road::drawHUDLine( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2, GLdouble t )
{
    glLineWidth( t );
    glPushMatrix();
    ;
    glBegin( GL_LINES );
    glVertex2d( x1, y1 );
    glVertex2d( x2, y2 );
    glEnd();
    glPopMatrix();

    glLineWidth( 1.0f );
}

void Road::drawHUDPiece( GLdouble, GLdouble, GLdouble rotAngleZ )
{
    glPushMatrix();
    glRotated( rotAngleZ, 0, 0, 1 );
    glColor4f( 1, 0, 0, 1 );
    glBegin( GL_TRIANGLES );
    glVertex2d( 0, 0 );
    glVertex2d( 0, 10 );
    glVertex2d( 2, 10 );
    glEnd();
    glPopMatrix();
}

void Road::drawCyberRings()
{
    glPushMatrix();
    const GLdouble sw = viewportWidth() / 2;
    const GLdouble sh = viewportHeight() / 2;
    glTranslated( sw, sh, 0 );
    for ( size_t i = 0; i < 3; i++ ) {
        glPushMatrix();
        glBindTexture( GL_TEXTURE_2D, m_cyberRingTexture[ i ] );
        glColor4fv( m_cyberRingColor[ i ] );
        glRotated( m_cyberRingRotation[ i ], 0, 0, 1 );
        glTranslated( m_maxDimention * -0.5, m_maxDimention * -0.5, 0 );
        glBegin( GL_QUADS );
        glTexCoord2f( 1, 1 );
        glVertex2d( m_maxDimention, m_maxDimention );
        glTexCoord2f( 0, 1 );
        glVertex2d( 0, m_maxDimention );
        glTexCoord2f( 0, 0 );
        glVertex2d( 0, 0 );
        glTexCoord2f( 1, 0 );
        glVertex2d( m_maxDimention, 0 );
        glEnd();
        glPopMatrix();
    }

    glPopMatrix();
}

void Road::drawCyberRingsMini()
{
    glPushMatrix();
    const GLdouble sw = viewportWidth() / 2;
    const GLdouble sh = viewportHeight() / 2 - 8;
    glTranslated( sw, sh, 0 );
    for ( size_t i = 0; i < 3; i++ ) {
        glPushMatrix();
        glEnable( GL_TEXTURE_2D );
        glEnable( GL_BLEND );
        glBindTexture( GL_TEXTURE_2D, m_cyberRingTexture[ i ] );
        glColor4f( m_hudColor4fv[ m_hudColor ][ 0 ], m_hudColor4fv[ m_hudColor ][ 1 ], m_hudColor4fv[ m_hudColor ][ 2 ], m_cyberRingColor[ i ][ 3 ] );
        glRotated( m_cyberRingRotation[ i ], 0, 0, 1 );
        glBegin( GL_QUADS );
        glTexCoord2f( 1, 1 );
        glVertex2d( 32, 32 );
        glTexCoord2f( 0, 1 );
        glVertex2d( -32, 32 );
        glTexCoord2f( 0, 0 );
        glVertex2d( -32, -32 );
        glTexCoord2f( 1, 0 );
        glVertex2d( 32, -32 );
        glEnd();
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_BLEND );
        glPopMatrix();
    }
    glPopMatrix();
}

void Road::drawHUDBar( GLuint x, GLuint y, GLuint w, GLuint h, GLuint current, GLuint max )
{
    glPushMatrix();
    glTranslated( x, y, 0 );
    glColor4fv( m_hudColor4fv[ m_hudColor ] );
    glBegin( GL_LINES );
    glVertex2d( -4, h + 4 );
    glVertex2d( w + 4, h + 4 );

    glVertex2d( w + 4, h + 4 );
    glVertex2d( w + 4, -4 );

    glVertex2d( w + 4, -4 );
    glVertex2d( -4, -4 );
    glEnd();
    glBegin( GL_QUADS );
    glColor3f(
        ( 1.0f - static_cast<GLfloat>( current ) / max ) + colorHalf( ( 1.0f - static_cast<GLfloat>( current ) / max ) )
        , static_cast<GLfloat>( current ) / max + colorHalf( static_cast<GLfloat>( current ) / max )
        , 0 );

    glVertex2d( 0, 0 );
    glVertex2d( w, 0 );
    glVertex2d( w, ( ( static_cast<GLdouble>( current ) / max ) * h ) );
    glVertex2d( 0, ( ( static_cast<GLdouble>( current ) / max ) * h ) );
    glEnd();
    glPopMatrix();
}

void Road::drawPauseText()
{
    glPushMatrix();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    drawCyberRings();
    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glColor4f( 0.1f, 0.4f, 0.9f, 0.8f );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();

    m_btnQuitMission.draw();
    glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );
    const char PAUSED[] = "PAUSED";
    const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( PAUSED ) ) / 2;
    m_fontPauseTxt->printText( posx, viewportHeight() - 128, PAUSED );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glPopMatrix();
}

void Road::renderHUD()
{
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glPushMatrix();
    setOrtho();

    glColor4fv( m_hudColor4fv[ m_hudColor ] );

    char hudmessage[ 48 ]{};
    std::snprintf( hudmessage, sizeof( hudmessage ), "Shots: %d", m_shotsDone );
    m_fontGuiTxt->printText( 320, viewportHeight() - 16, hudmessage );

    std::snprintf( hudmessage, sizeof( hudmessage ), "SCORE: %d", m_jet->score() );
    m_fontGuiTxt->printText( 64, viewportHeight() - 100, hudmessage );

    /*radar*/
    GLfloat matrice[ 16 ]{};
    m_jet->quat().createMatrix( matrice );

    glPushMatrix();
    glEnable( GL_DEPTH_TEST );
    glTranslated( viewportWidth() - 80, 80, 0 );

    glMultMatrixf( matrice );

    glBegin( GL_LINES );
    glVertex3d( 24, 24, 24 );
    glVertex3d( -24, 24, 24 );
    glVertex3d( 24, -24, 24 );
    glVertex3d( -24, -24, 24 );
    glVertex3d( 24, 24, -24 );
    glVertex3d( -24, 24, -24 );
    glVertex3d( 24, -24, -24 );
    glVertex3d( -24, -24, -24 );

    glVertex3d( 24, 24, 24 );
    glVertex3d( 24, 24, -24 );
    glVertex3d( 24, -24, 24 );
    glVertex3d( 24, -24, -24 );
    glVertex3d( -24, 24, 24 );
    glVertex3d( -24, 24, -24 );
    glVertex3d( -24, -24, 24 );
    glVertex3d( -24, -24, -24 );

    glVertex3d( 24, 24, 24 );
    glVertex3d( 24, -24, 24 );
    glVertex3d( 24, 24, -24 );
    glVertex3d( 24, -24, -24 );
    glVertex3d( -24, 24, 24 );
    glVertex3d( -24, -24, 24 );
    glVertex3d( -24, 24, -24 );
    glVertex3d( -24, -24, -24 );
    glEnd();

    glColor3f( 0.3, 0.3, 1 );
    glBegin( GL_LINE_LOOP );
    for ( size_t i = 0; i < m_radar->segments(); i++ ) {
        glVertex3d( m_radar->x( i ), m_radar->y( i ), 0 );
    }
    glEnd();
    glColor3f( 0, 1, 0 );
    glBegin( GL_LINE_LOOP );
    for ( size_t i = 0; i < m_radar->segments(); i++ ) {
        glVertex3d( m_radar->x( i ), 0, m_radar->y( i ) );
    }
    glEnd();
    glColor3f( 1, 0, 0 );
    glBegin( GL_LINE_LOOP );
    for ( size_t i = 0; i < m_radar->segments(); i++ ) {
        glVertex3d( 0, m_radar->x( i ), m_radar->y( i ) );
    }
    glEnd();
    for ( const Enemy* e : m_enemies ) {
        e->drawRadarPosition( m_jet->position(), 92 );
    }

    glDisable( GL_DEPTH_TEST );
    Vertex cursor = m_jet->direction() * 92;
    glLineWidth( 3 );
    glColor4f( 1, 1, 0, 0.9 );
    glBegin( GL_LINES );
    glVertex3d( 0, 0, 0 );

    glVertex3d( cursor.x, cursor.y, cursor.z );
    glEnd();
    glLineWidth( 1 );

    glPopMatrix();

    glColor4fv( m_hudColor4fv[ m_hudColor ] );

    glPushMatrix();
    glTranslated( 32, viewportHeight() / 2, 0 );
    {
        std::string str{ "SPEED: " };
        str += std::to_string( static_cast<int>( m_jet->speed() ) * 270 );
        m_fontGuiTxt->printText( 38, 0, str.c_str() );
    }
    glPushMatrix();
    glRotated( m_speedAnim, 0, 0, 1 );
    glBegin( GL_POLYGON );
    glVertex2d( -3, 0 );
    glVertex2d( 3, 0 );
    glVertex2d( 12, 24 );
    glVertex2d( 0, 26.5 );
    glVertex2d( -12, 24 );
    glEnd();
    glBegin( GL_POLYGON );
    glVertex2d( -12, -24 );
    glVertex2d( 0, -26.5 );
    glVertex2d( 12, -24 );
    glVertex2d( 3, 0 );
    glVertex2d( -3, 0 );
    glEnd();
    glPopMatrix();
    glBegin( GL_LINE_LOOP );
    for ( size_t i = 0; i < m_speedFanRing->segments(); i++ ) {
        glVertex2d( m_speedFanRing->x( i ), m_speedFanRing->y( i ) );
    }
    glEnd();
    glPopMatrix();

    drawCyberRingsMini();

    drawHUDBar( 12, 12, 36, 96, m_jet->energy(), 100 );
    drawHUDBar( 64, 12, 36, 96, m_jet->health(), 100 );

    glColor4fv( m_hudColor4fv[ m_hudColor ] );

    {
        std::string msg{ "FPS done: " };
        msg += std::to_string( m_fps );
        msg += ", calculated: ";
        msg += std::to_string( m_calculatedFPS );
        const double textMid = static_cast<double>( m_fontGuiTxt->textLength( "FPS done: XX, calculated: xxxx.xx" ) ) / 2;
        const double posx = viewportWidth() / 2 - textMid;
        m_fontGuiTxt->printText( posx, viewportHeight() - 28, msg.c_str() );
    }
    m_fontPauseTxt->printText( 10, 102, "ENG" );
    m_fontPauseTxt->printText( 66, 102, "HP" );

    glPopMatrix();
}

void Road::render3D()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_FOG );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    setPerspective( m_angle + m_jet->speed() * 6 );

    const double cX = -m_jet->x();
    const double cY = -m_jet->y();
    const double cZ = -m_jet->z();

    glPushMatrix();
    glTranslated( 0, -0.225, -1 );
    glPushMatrix();
    GLfloat matrice[ 16 ]{};
    m_jet->rotation().createMatrix( matrice );
    glMultMatrixf( matrice );
    glTranslated( cX, cY, cZ );
    m_map->draw();

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( const Enemy* it : m_enemies ) {
            it->draw();
        }
    }
    glEnable( GL_BLEND );
    glLineWidth( 2 );
    {
        std::lock_guard<std::mutex> lg( m_mutexBullet );
        for ( const Bullet* it : m_bullets ) {
            it->draw();
        }
    }
    {
        std::lock_guard<std::mutex> lg1( m_mutexEnemy );
        std::lock_guard<std::mutex> lg2( m_mutexEnemyBullet );
        for ( const Bullet* it : m_enemyBullets ) {
            it->draw();
        }
    }
    glLineWidth( 1 );
    glDisable( GL_BLEND );
    glPopMatrix();
    m_jet->draw();
    glPopMatrix();
}

void Road::drawMainMenu()
{
    glPushMatrix();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    setOrtho();

    drawClouds();
    drawCyberRings();

    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glColor4f( 0.1f, 0.4f, 0.9f, 0.8f );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();

    m_btnSelectMission.draw();
    m_btnExit.draw();
    m_btnCustomize.draw();

    glPopMatrix();
}

void Road::drawClouds() const
{
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glBindTexture( GL_TEXTURE_2D, m_menuBackground );
    glBegin( GL_QUADS );
    glColor4f( 0.1f, 0.4f, 0.9f, 1 );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glColor4f( 0.9f, 0.4f, 0.1f, 1 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glColor4f( 0.1f, 0.9f, 0.4f, 1 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glColor4f( 0.4f, 0.1f, 0.9f, 1 );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();
    glBindTexture( GL_TEXTURE_2D, m_menuBackgroundOverlay );
    glColor4f( 1, 1, 1, m_alphaValue );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();

    glBindTexture( GL_TEXTURE_2D, m_starfieldTexture );
    glColor4f( 1, 1, 1, m_alphaValue );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( m_maxDimention, 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( m_maxDimention, m_maxDimention );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, m_maxDimention );
    glEnd();
    glPopMatrix();
}

void Road::winScreen()
{
    gameScreen();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    drawCyberRings();
    glColor4fv( m_hudColor4fv[ 0 ] );
    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();
    glPopMatrix();

    glColor3f( 1, 1, 1 );
    {
        constexpr static char missionOK[] = "MISSION SUCCESSFUL";
        m_fontBig->printText( viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( missionOK ) ) / 2, viewportHeight() - 128, missionOK );

        std::string str{ "Your score: " };
        str += std::to_string( m_jet->score() );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( str.c_str() ) ) / 2;
        m_fontBig->printText( posx, viewportHeight() - 128 - 36, str.c_str() );
    }
    m_btnReturnToMissionSelection.draw();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Road::deadScreen()
{
    gameScreen();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    drawCyberRings();
    glColor4fv( m_hudColor4fv[ 2 ] );
    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();
    glPopMatrix();

    glColor3f( 1, 1, 1 );
    {
        constexpr static char missionOK[] = "MISSION FAILED";
        m_fontBig->printText( viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( missionOK ) ) / 2, viewportHeight() - 128, missionOK );

        std::string str{ "Your score: " };
        str += std::to_string( m_jet->score() );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( str.c_str() ) ) / 2;
        m_fontBig->printText( posx, viewportHeight() - 128 - 36, str.c_str() );
    }
    m_btnReturnToMissionSelection.draw();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Road::missionSelectionScreen()
{
    glPushMatrix();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    setOrtho();
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glPushMatrix();
    {
        MapProto& map = m_mapsContainer[ m_currentMap ];
        if ( map.preview_image == 0 ) {
            map.preview_image = loadTexture( map.preview_image_location.c_str() );
        }
        glBindTexture( GL_TEXTURE_2D, map.preview_image );
    }

    glBegin( GL_QUADS );
    glColor3f( 1, 1, 1 );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( m_maxDimention, 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( m_maxDimention, m_maxDimention );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, m_maxDimention );
    glEnd();
    {
        std::string str{ "Map: " };
        str += m_mapsContainer.at( m_currentMap ).name;
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->printText( posx, viewportHeight() - 128, str.c_str() );
    }
    {
        std::string str{ "Enemies: " };
        str += std::to_string( m_mapsContainer.at( m_currentMap ).enemies );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->textLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->printText( posx, viewportHeight() - 148, str.c_str() );
    }

    glPopMatrix();

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    drawCyberRings();
    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glBegin( GL_QUADS );
    glColor3f( 0, 0.75, 1 );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();

    m_btnStartMission.draw();
    m_btnReturnToMainMenu.draw();
    m_btnNextMap.draw();
    m_btnPrevMap.draw();

    glPopMatrix();
}

void Road::gameScreenBriefing()
{
    gameScreen();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    drawCyberRings();
    glColor3f( 1, 1, 1 );
    m_fontPauseTxt->printText( 192, viewportHeight() - 292, "Movement: AWSD QE" );
    m_fontPauseTxt->printText( 192, viewportHeight() - 310, "Speed controll: UO" );
    m_fontPauseTxt->printText( 192, viewportHeight() - 328, "Weapons: JKL" );
    m_fontPauseTxt->printText( 192, viewportHeight() - 346, "Targeting: I" );
    m_fontPauseTxt->printText( 192, viewportHeight() - 380, "Press space to launch..." );

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glBegin( GL_QUADS );
    glColor3f( 0, 0.75, 1 );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();
    m_btnGO.draw();
    glPopMatrix();
}

void Road::screenCustomize()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    setOrtho();
    drawClouds();
    drawCyberRings();
    glBindTexture( GL_TEXTURE_2D, m_hudTex );
    glBegin( GL_QUADS );
    glColor3f( 0, 0.75, 1 );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( viewportWidth(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( viewportWidth(), viewportHeight() );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, viewportHeight() );
    glEnd();
    glColor3f( 1, 1, 1 );
    {
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->textLength( m_jetsContainer.at( m_currentJet ).name.c_str() ) ) / 2;
        m_fontBig->printText( posx, viewportHeight() - 64, m_jetsContainer.at( m_currentJet ).name.c_str() );
    }
    setPerspective( m_angle );
    glPushMatrix();
    glTranslated( 0, -0.1, -1.25 );
    glRotated( 15, 1, 0, 0 );
    glRotated( m_modelRotation, 0, 1, 0 );
    glEnable( GL_DEPTH_TEST );
    m_previewModel.draw();
    glDisable( GL_DEPTH_TEST );
    glPopMatrix();
    setOrtho();
    m_btnNextJet.draw();
    m_btnPrevJet.draw();
    m_btnCustomizeReturn.draw();
    m_btnWeap1.draw();
    m_btnWeap2.draw();
    m_btnWeap3.draw();
}

void Road::drawBullets()
{
}
