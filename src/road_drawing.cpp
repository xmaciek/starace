#include "road.hpp"

void Road::GameScreen()
{
    Render3D();
    RenderHUD();
    FramesDone++;
    tempFPS += ( SDL_GetTicks() - timeS );
    if ( m_timePassed < time( nullptr ) ) {
        FPS = FramesDone;
        CalculatedFPS = 1000.0f / ( tempFPS / FramesDone ); // FramesDone);
        tempFPS = 0;
        FramesDone = 0;
        m_timePassed++;
    }
}

void Road::GameScreenPaused()
{
    GameScreen();
    DrawPauseText();
}

void Road::DrawAxis()
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

void Road::DrawLine( GLdouble X, GLdouble Y )
{
    glPushMatrix();
    glBegin( GL_LINES );
    glColor4f( 1, 0.4f, 0, 0 );
    glVertex3d( X, Y, -100 );
    glColor4f( 1, 0.4f, 0, 1 );
    glVertex3d( X, Y, 1 );
    glEnd();
    glPopMatrix();
}

void Road::DrawHudRect( GLdouble X, GLdouble Y, GLdouble W, GLdouble H )
{
    glPushMatrix();
    glTranslated( X, Y, 0 );
    glBegin( GL_QUADS );
    glVertex2d( 0, 0 );
    glVertex2d( 0, H );
    glVertex2d( W, H );
    glVertex2d( W, 0 );
    glEnd();
    glPopMatrix();
}

void Road::DrawHUDLine( GLdouble X1, GLdouble Y1, GLdouble X2, GLdouble Y2, GLdouble T )
{
    glLineWidth( T );
    glPushMatrix();
    ;
    glBegin( GL_LINES );
    glVertex2d( X1, Y1 );
    glVertex2d( X2, Y2 );
    glEnd();
    glPopMatrix();

    glLineWidth( 1.0f );
}

void Road::DrawHUDPiece( GLdouble, GLdouble, GLdouble RotAngleZ )
{
    glPushMatrix();
    glRotated( RotAngleZ, 0, 0, 1 );
    glColor4f( 1, 0, 0, 1 );
    glBegin( GL_TRIANGLES );
    glVertex2d( 0, 0 );
    glVertex2d( 0, 10 );
    glVertex2d( 2, 10 );
    glEnd();
    glPopMatrix();
}

void Road::DrawCyberRings()
{
    glPushMatrix();
    const GLdouble sw = viewportWidth() / 2;
    const GLdouble sh = viewportHeight() / 2;
    glTranslated( sw, sh, 0 );
    for ( size_t i = 0; i < 3; i++ ) {
        glPushMatrix();
        glBindTexture( GL_TEXTURE_2D, cyber_ring_texture[ i ] );
        glColor4fv( cyber_ring_color[ i ] );
        glRotated( cyber_ring_rotation[ i ], 0, 0, 1 );
        glTranslated( max_dimention * -0.5, max_dimention * -0.5, 0 );
        glBegin( GL_QUADS );
        glTexCoord2f( 1, 1 );
        glVertex2d( max_dimention, max_dimention );
        glTexCoord2f( 0, 1 );
        glVertex2d( 0, max_dimention );
        glTexCoord2f( 0, 0 );
        glVertex2d( 0, 0 );
        glTexCoord2f( 1, 0 );
        glVertex2d( max_dimention, 0 );
        glEnd();
        glPopMatrix();
    } /*
    glPushMatrix();
      glBindTexture(GL_TEXTURE_2D, cyber_ring_texture[3]);
      glColor4fv(cyber_ring_color[3]);
      glRotated(60,1,0,0);
      glRotated(cyber_ring_rotation[3],0,0,1);
      glTranslated(max_dimention*-0.5, max_dimention*-0.5, 0);
      glBegin(GL_QUADS);
        glTexCoord2f(1,1); glVertex2d(max_dimention, max_dimention);
        glTexCoord2f(0,1); glVertex2d(0, max_dimention);
        glTexCoord2f(0,0); glVertex2d(0, 0);
        glTexCoord2f(1,0); glVertex2d(max_dimention, 0);
      glEnd();   
    glPopMatrix();*/

    glPopMatrix();
}

void Road::DrawCyberRingsMini()
{
    glPushMatrix();
    const GLdouble sw = viewportWidth() / 2;
    const GLdouble sh = viewportHeight() / 2 - 8;
    glTranslated( sw, sh, 0 );
    for ( size_t i = 0; i < 3; i++ ) {
        glPushMatrix();
        glEnable( GL_TEXTURE_2D );
        glEnable( GL_BLEND );
        glBindTexture( GL_TEXTURE_2D, cyber_ring_texture[ i ] );
        glColor4f( HUD_Color_4fv[ HUD_Color ][ 0 ], HUD_Color_4fv[ HUD_Color ][ 1 ], HUD_Color_4fv[ HUD_Color ][ 2 ], cyber_ring_color[ i ][ 3 ] );
        glRotated( cyber_ring_rotation[ i ], 0, 0, 1 );
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

void Road::DrawHUDBar( const GLuint& X, const GLuint& Y, const GLuint& W, const GLuint& H, const GLuint& Current, const GLuint& Max )
{
    glPushMatrix();
    glTranslated( X, Y, 0 );
    glColor4fv( HUD_Color_4fv[ HUD_Color ] );
    glBegin( GL_LINES );
    glVertex2d( -4, H + 4 );
    glVertex2d( W + 4, H + 4 );

    glVertex2d( W + 4, H + 4 );
    glVertex2d( W + 4, -4 );

    glVertex2d( W + 4, -4 );
    glVertex2d( -4, -4 );
    glEnd();
    //     DrawHUDLine(X-4, viewportHeight()-8, X+W+4, viewportHeight()-8, 2);
    //     DrawHUDLine(X+W+4, viewportHeight()-8, X+W+4, viewportHeight()-112, 2);
    //     DrawHUDLine(X+W+4, viewportHeight()-112, X-4, viewportHeight()-112, 2);
    glBegin( GL_QUADS );
    //       glColor4f(1.0f, (GLfloat)Current/Max, 0, 0.8f);
    //     (1.0f-(GLfloat)health/1000)+colorhalf((1.0f-(GLfloat)health/1000)),
    //           glColor4f(1-(m_jet->energy/100) + colorhalf(1-m_jet->energy/100), colorhalf(m_jet->energy/100)+(GLfloat)m_jet->energy/100, 0, 0.8);
    glColor3f( ( 1.0f - static_cast<GLfloat>( Current ) / Max ) + colorhalf( ( 1.0f - static_cast<GLfloat>( Current ) / Max ) ), static_cast<GLfloat>( Current ) / Max + colorhalf( static_cast<GLfloat>( Current ) / Max ), 0 );

    glVertex2d( 0, 0 );
    glVertex2d( W, 0 );
    glVertex2d( W, ( ( static_cast<GLdouble>( Current ) / Max ) * H ) );
    glVertex2d( 0, ( ( static_cast<GLdouble>( Current ) / Max ) * H ) );
    glEnd();
    glPopMatrix();
}

void Road::DrawPauseText()
{
    glPushMatrix();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    DrawCyberRings();
    glBindTexture( GL_TEXTURE_2D, HUDtex );
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

    //         m_btnChangeFiltering.Draw();
    m_btnQuitMission.Draw();
    glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );
    const char PAUSED[] = "PAUSED";
    const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->GetTextLength( PAUSED ) ) / 2;
    m_fontPauseTxt->PrintTekst( posx, viewportHeight() - 128, PAUSED );
    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
    glPopMatrix();
}

void Road::RenderHUD()
{
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glPushMatrix();
    SetOrtho();

    glColor4fv( HUD_Color_4fv[ HUD_Color ] );

    char hudmessage[ 48 ]{};
    std::snprintf( hudmessage, sizeof( hudmessage ), "Shots: %d", ShotsDone );
    m_fontGuiTxt->PrintTekst( 320, viewportHeight() - 16, hudmessage );

    std::snprintf( hudmessage, sizeof( hudmessage ), "SCORE: %d", m_jet->GetScore() );
    m_fontGuiTxt->PrintTekst( 64, viewportHeight() - 100, hudmessage );

    /*radar*/
    GLfloat matrice[ 16 ];
    m_jet->quat().CreateMatrix( matrice );

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
    for ( size_t i = 0; i < m_radar->GetSegments(); i++ ) {
        glVertex3d( m_radar->GetX( i ), m_radar->GetY( i ), 0 );
    }
    glEnd();
    glColor3f( 0, 1, 0 );
    glBegin( GL_LINE_LOOP );
    for ( size_t i = 0; i < m_radar->GetSegments(); i++ ) {
        glVertex3d( m_radar->GetX( i ), 0, m_radar->GetY( i ) );
    }
    glEnd();
    glColor3f( 1, 0, 0 );
    glBegin( GL_LINE_LOOP );
    for ( size_t i = 0; i < m_radar->GetSegments(); i++ ) {
        glVertex3d( 0, m_radar->GetX( i ), m_radar->GetY( i ) );
    }
    glEnd();
    for ( const Enemy* e : m_enemies ) {
        e->DrawRadarPosition( m_jet->GetPosition(), 92 );
    }

    glDisable( GL_DEPTH_TEST );
    Vertex cursor = m_jet->GetDirection() * 92;
    glLineWidth( 3 );
    glColor4f( 1, 1, 0, 0.9 );
    glBegin( GL_LINES );
    glVertex3d( 0, 0, 0 );

    glVertex3d( cursor.x, cursor.y, cursor.z );
    glEnd();
    glLineWidth( 1 );

    glPopMatrix();

    glColor4fv( HUD_Color_4fv[ HUD_Color ] );

    glPushMatrix();
    glTranslated( 32, viewportHeight() / 2, 0 );
    {
        std::string str{ "SPEED: " };
        str += std::to_string( static_cast<int>( m_jet->GetSpeed() ) * 270 );
        m_fontGuiTxt->PrintTekst( 38, 0, str.c_str() );
    }
    glPushMatrix();
    glRotated( speed_anim, 0, 0, 1 );
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
    for ( size_t i = 0; i < m_speedFanRing->GetSegments(); i++ ) {
        glVertex2d( m_speedFanRing->GetX( i ), m_speedFanRing->GetY( i ) );
    }
    glEnd();
    glPopMatrix();

    DrawCyberRingsMini();

    DrawHUDBar( 12, 12, 36, 96, m_jet->energy(), 100 );
    DrawHUDBar( 64, 12, 36, 96, m_jet->GetHealth(), 100 );

    glColor4fv( HUD_Color_4fv[ HUD_Color ] );

    {
        std::string msg{ "FPS done: " };
        msg += std::to_string( FPS );
        msg += ", calculated: ";
        msg += std::to_string( CalculatedFPS );
        const double textMid = static_cast<double>( m_fontGuiTxt->GetTextLength( "FPS done: XX, calculated: xxxx.xx" ) ) / 2;
        const double posx = viewportWidth() / 2 - textMid;
        m_fontGuiTxt->PrintTekst( posx, viewportHeight() - 28, msg.c_str() );
    }
    m_fontPauseTxt->PrintTekst( 10, 102, "ENG" );
    m_fontPauseTxt->PrintTekst( 66, 102, "HP" );

    glPopMatrix();
}

void Road::Render3D()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_FOG );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    SetPerspective( angle + m_jet->GetSpeed() * 6 );

    const double cX = -m_jet->getX();
    const double cY = -m_jet->getY();
    const double cZ = -m_jet->getZ();

    glPushMatrix();
    glTranslated( 0, -0.225, -1 );
    glPushMatrix();
    GLfloat matrice[ 16 ];
    m_jet->rotation().CreateMatrix( matrice );
    glMultMatrixf( matrice );
    glTranslated( cX, cY, cZ );
    m_map->Draw();

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( const Enemy* it : m_enemies ) {
            it->Draw();
        }
    }
    glEnable( GL_BLEND );
    glLineWidth( 2 );
    {
        std::lock_guard<std::mutex> lg( m_mutexBullet );
        for ( const Bullet* it : m_bullets ) {
            it->Draw();
        }
    }
    {
        std::lock_guard<std::mutex> lg1( m_mutexEnemy );
        std::lock_guard<std::mutex> lg2( m_mutexEnemyBullet );
        for ( const Bullet* it : m_enemyBullets ) {
            it->Draw();
        }
    }
    glLineWidth( 1 );
    glDisable( GL_BLEND );
    glPopMatrix();
    m_jet->Draw();
    glPopMatrix();
}

void Road::DrawMainMenu()
{
    glPushMatrix();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    SetOrtho();

    DrawClouds();
    DrawCyberRings();

    glBindTexture( GL_TEXTURE_2D, HUDtex );
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

    m_btnSelectMission.Draw();
    m_btnExit.Draw();
    m_btnCustomize.Draw();

    glPopMatrix();
}

void Road::DrawClouds() const
{
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glBindTexture( GL_TEXTURE_2D, menu_background );
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
    glBindTexture( GL_TEXTURE_2D, menu_background_overlay );
    glColor4f( 1, 1, 1, alpha_value );
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

    glBindTexture( GL_TEXTURE_2D, starfield_texture );
    glColor4f( 1, 1, 1, alpha_value );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( max_dimention, 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( max_dimention, max_dimention );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, max_dimention );
    glEnd();
    glPopMatrix();
}

void Road::WinScreen()
{
    GameScreen();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    DrawCyberRings();
    glColor4fv( HUD_Color_4fv[ 0 ] );
    glBindTexture( GL_TEXTURE_2D, HUDtex );
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
        m_fontBig->PrintTekst( viewportWidth() / 2 - static_cast<double>( m_fontBig->GetTextLength( missionOK ) ) / 2, viewportHeight() - 128, missionOK );

        std::string str{ "Your score: " };
        str += std::to_string( m_jet->GetScore() );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->GetTextLength( str.c_str() ) ) / 2;
        m_fontBig->PrintTekst( posx, viewportHeight() - 128 - 36, str.c_str() );
    }
    m_btnReturnToMissionSelection.Draw();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Road::DeadScreen()
{
    GameScreen();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    DrawCyberRings();
    glColor4fv( HUD_Color_4fv[ 2 ] );
    glBindTexture( GL_TEXTURE_2D, HUDtex );
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
        m_fontBig->PrintTekst( viewportWidth() / 2 - static_cast<double>( m_fontBig->GetTextLength( missionOK ) ) / 2, viewportHeight() - 128, missionOK );

        std::string str{ "Your score: " };
        str += std::to_string( m_jet->GetScore() );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->GetTextLength( str.c_str() ) ) / 2;
        m_fontBig->PrintTekst( posx, viewportHeight() - 128 - 36, str.c_str() );
    }
    m_btnReturnToMissionSelection.Draw();

    glDisable( GL_TEXTURE_2D );
    glDisable( GL_BLEND );
}

void Road::MissionSelectionScreen()
{
    glPushMatrix();
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    SetOrtho();
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glPushMatrix();
    {
        MapProto& map = m_mapsContainer[ current_map ];
        if ( map.preview_image == 0 ) {
            map.preview_image = LoadTexture( map.preview_image_location.c_str() );
        }
        glBindTexture( GL_TEXTURE_2D, map.preview_image );
    }

    glBegin( GL_QUADS );
    glColor3f( 1, 1, 1 );
    glTexCoord2f( 0, 0 );
    glVertex2d( 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex2d( max_dimention, 0 );
    glTexCoord2f( 1, 1 );
    glVertex2d( max_dimention, max_dimention );
    glTexCoord2f( 0, 1 );
    glVertex2d( 0, max_dimention );
    glEnd();
    {
        std::string str{ "Map: " };
        str += m_mapsContainer.at( current_map ).name;
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->GetTextLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->PrintTekst( posx, viewportHeight() - 128, str.c_str() );
    }
    {
        std::string str{ "Enemies: " };
        str += std::to_string( m_mapsContainer.at( current_map ).enemies );
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontPauseTxt->GetTextLength( str.c_str() ) ) / 2;
        m_fontPauseTxt->PrintTekst( posx, viewportHeight() - 148, str.c_str() );
    }

    glPopMatrix();

    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    DrawCyberRings();
    glBindTexture( GL_TEXTURE_2D, HUDtex );
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

    m_btnStartMission.Draw();
    m_btnReturnToMainMenu.Draw();
    m_btnNextMap.Draw();
    m_btnPrevMap.Draw();

    glPopMatrix();
}

void Road::GameScreenBriefing()
{
    GameScreen();
    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    DrawCyberRings();
    glColor3f( 1, 1, 1 );
    m_fontPauseTxt->PrintTekst( 192, viewportHeight() - 292, "Movement: AWSD QE" );
    m_fontPauseTxt->PrintTekst( 192, viewportHeight() - 310, "Speed controll: UO" );
    m_fontPauseTxt->PrintTekst( 192, viewportHeight() - 328, "Weapons: JKL" );
    m_fontPauseTxt->PrintTekst( 192, viewportHeight() - 346, "Targeting: I" );
    m_fontPauseTxt->PrintTekst( 192, viewportHeight() - 380, "Press space to launch..." );

    glEnable( GL_BLEND );
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, HUDtex );
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
    m_btnGO.Draw();
    glPopMatrix();
}

void Road::ScreenCustomize()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    SetOrtho();
    DrawClouds();
    DrawCyberRings();
    glBindTexture( GL_TEXTURE_2D, HUDtex );
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
        const double posx = viewportWidth() / 2 - static_cast<double>( m_fontBig->GetTextLength( m_jetsContainer.at( current_jet ).name.c_str() ) ) / 2;
        m_fontBig->PrintTekst( posx, viewportHeight() - 64, m_jetsContainer.at( current_jet ).name.c_str() );
    }
    SetPerspective( angle );
    glPushMatrix();
    glTranslated( 0, -0.1, -1.25 );
    glRotated( 15, 1, 0, 0 );
    glRotated( model_rotation, 0, 1, 0 );
    glEnable( GL_DEPTH_TEST );
    m_previewModel.Draw();
    glDisable( GL_DEPTH_TEST );
    glPopMatrix();
    SetOrtho();
    m_btnNextJet.Draw();
    m_btnPrevJet.Draw();
    m_btnCustomizeReturn.Draw();
    m_btnWeap1.Draw();
    m_btnWeap2.Draw();
    m_btnWeap3.Draw();
}

void Road::DrawBullets()
{
}
