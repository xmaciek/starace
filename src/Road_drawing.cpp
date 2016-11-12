#include "Road.h"



  void Road::GameScreen() {
    Render3D();
    RenderHUD();
    FramesDone++;
    tempFPS += (SDL_GetTicks() - timeS);
    if (TimePassed<time(NULL)) {
      FPS =  FramesDone; 
      CalculatedFPS = 1000.0f/ (tempFPS/FramesDone); // FramesDone);
      tempFPS = 0;
      FramesDone = 0; 
      TimePassed++;
    }
  }

void Road::GameScreenPaused()
{
    GameScreen();
    DrawCyberRings();
    drawHudGlow();
    SHADER::pushMatrix();
        SetOrtho();
        btnQuitMission.Draw();
        const char txtPaused[] = "PAUSED";
        const uint32_t textLength = font_pause_txt->GetTextLength( txtPaused );
        font_pause_txt->PrintTekst( SCREEN_WIDTH / 2 - textLength / 2, SCREEN_HEIGHT - 128, txtPaused );
    SHADER::popMatrix();
}

  
  
  
  
//   void Road::DrawCrosshair() {
//     glColor3f(1,1,1);
//     glBegin(GL_LINES);
//       glVertex3f(jetX-0.05f, jetY, -1.5f);
//       glVertex3f(jetX+0.05f, jetY, -1.5f);
//    
//       glVertex3f(jetX, jetY-0.05f, -1.5f);
//       glVertex3f(jetX, jetY+0.05f, -1.5f);
//     glEnd();
//   }
  
  void Road::DrawAxis() {
    SHADER::pushMatrix();
      glBegin(GL_LINES);
        glColor3f(1,0,0);
        glVertex3d(-0.1,0,0);
        glVertex3d(0.1,0,0);
    
        glColor3f(0,1,0);
        glVertex3d(0,-0.1,0);
        glVertex3d(0,0.1,0);
    
        glColor3f(0,0,1);
        glVertex3d(0,0,-0.1);
        glVertex3d(0,0,0.1);
      glEnd();
    SHADER::popMatrix();
  }
  
  
  /* HUD elements */
  
  
  
  


  void Road::DrawLine(GLdouble X, GLdouble Y) {
    SHADER::pushMatrix();
      glBegin(GL_LINES);
        SHADER::setColor(1, 0.4f, 0, 0);
        glVertex3d(X, Y, -100);
        SHADER::setColor(1, 0.4f, 0, 1);
        glVertex3d(X, Y, 1);
      glEnd();
    SHADER::popMatrix();
  }





  void Road::DrawHudRect(GLdouble X, GLdouble Y, GLdouble W, GLdouble H) {
    SHADER::pushMatrix();
      SHADER::translate(X,Y,0);
      glBegin(GL_QUADS);
        glVertex2d(0,0);
        glVertex2d(0,H);
        glVertex2d(W,H);
        glVertex2d(W,0);
      glEnd();
    SHADER::popMatrix();
  }

  void Road::DrawHUDLine(GLdouble X1, GLdouble Y1, GLdouble X2, GLdouble Y2, GLdouble T) {
    glLineWidth(T);
    SHADER::pushMatrix();;
      glBegin(GL_LINES);
        glVertex2d(X1, Y1);
        glVertex2d(X2, Y2);
      glEnd();
    SHADER::popMatrix();

    glLineWidth(1.0f);
  }

  void Road::DrawHUDPiece(GLdouble X, GLdouble Y, GLdouble RotAngleZ) {
    SHADER::pushMatrix();
      SHADER::rotate(RotAngleZ,0,0,1);
      SHADER::setColor(1,0,0,1);
      glBegin(GL_TRIANGLES);
        glVertex2d(0,0);
        glVertex2d(0,10);
        glVertex2d(2,10);
      glEnd();
    SHADER::popMatrix();
  }

void Road::drawHudGlow()
{
    if ( !m_textureHudGlow ) {
        loadHudGlow();
    }
    SHADER::pushMatrix();
    const double wr = 0.5 * SCREEN_WIDTH / max_dimention;
    const double hr = 0.5 * SCREEN_HEIGHT / max_dimention;
    SHADER::setOrtho( -wr, wr, -hr, hr );
    SHADER::setMaterial( m_textureHudGlow );
    SHADER::setTextureCoord( m_bufferHudGlowUV );
    SHADER::drawBuffer( m_bufferHudGlow );
    SHADER::popMatrix();
}

void Road::DrawCyberRings()
{
    SHADER::pushMatrix();

    const double wr = 0.5 * SCREEN_WIDTH / max_dimention;
    const double hr = 0.5 * SCREEN_HEIGHT / max_dimention;
    SHADER::setOrtho( -wr, wr, -hr, hr );

    static const uint32_t ring = SHADER::getQuad( -0.5, -0.5, 0.5, 0.5 );
    static const uint32_t ringCoord = SHADER::getQuadTextureCoord( 0, 0, 1, 1 );

    SHADER::setTextureCoord( ringCoord );

    SHADER::pushMatrix();
        m_ringTextureA.use();
        SHADER::rotate( cyber_ring_rotation[0], 0, 0, 1 );
        SHADER::draw( GL_TRIANGLES, ring, 6 );
    SHADER::popMatrix();

    SHADER::pushMatrix();
        m_ringTextureB.use();
        SHADER::rotate( cyber_ring_rotation[1], 0, 0, 1 );
        SHADER::draw( GL_TRIANGLES, ring, 6 );
    SHADER::popMatrix();

    SHADER::pushMatrix();
        m_ringTextureC.use();
        SHADER::rotate( cyber_ring_rotation[2], 0, 0, 1 );
        SHADER::draw( GL_TRIANGLES, ring, 6 );
    SHADER::popMatrix();

    SHADER::popMatrix();
}

void Road::DrawCyberRingsMini()
{
    SHADER::pushMatrix();
    SHADER::translate( SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0 );
    static const uint32_t ring = SHADER::getQuad( -32, -32, 32, 32 );
    static const uint32_t ringCoord = SHADER::getQuadTextureCoord( 0, 0, 1, 1 );

    SHADER::setTextureCoord( ringCoord );

    SHADER::pushMatrix();
        m_ringTextureA.use();
        SHADER::rotate( cyber_ring_rotation[0], 0, 0, 1 );
        SHADER::draw( GL_TRIANGLES, ring, 6 );
    SHADER::popMatrix();

    SHADER::pushMatrix();
        m_ringTextureB.use();
        SHADER::rotate( cyber_ring_rotation[1], 0, 0, 1 );
        SHADER::draw( GL_TRIANGLES, ring, 6 );
    SHADER::popMatrix();

    SHADER::pushMatrix();
        m_ringTextureC.use();
        SHADER::rotate( cyber_ring_rotation[2], 0, 0, 1 );
        SHADER::draw( GL_TRIANGLES, ring, 6 );
    SHADER::popMatrix();

    SHADER::popMatrix();
}

static Buffer getFanRotorRing()
{
    std::vector<double> arr;
    Circle circle( 32, 26.5 );
    for ( int i=0; i<circle.segments(); i++ ) {
        arr.push_back( circle.x( i ) );
        arr.push_back( circle.y( i ) );
        arr.push_back( 0 );
    }
    return SHADER::makeBuffer( arr, Buffer::LineLoop );
}

static Buffer getFanRotorPetals()
{
    std::vector<double> arr;

    arr.push_back( 0 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( 3 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( 12 );
    arr.push_back( 24 );
    arr.push_back( 0 );

    arr.push_back( 0 );
    arr.push_back( 26.5 );
    arr.push_back( 0 );

    arr.push_back( -12 );
    arr.push_back( 24 );
    arr.push_back( 0 );

    arr.push_back( -3 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    arr.push_back( -12 );
    arr.push_back( -24 );
    arr.push_back( 0 );

    arr.push_back( 0 );
    arr.push_back( -26.5 );
    arr.push_back( 0 );

    arr.push_back( 12 );
    arr.push_back( -24 );
    arr.push_back( 0 );

    arr.push_back( 3 );
    arr.push_back( 0 );
    arr.push_back( 0 );

    return SHADER::makeBuffer( arr, Buffer::TriangleFan );
}

  void Road::DrawHUDBar(const GLuint &X, const GLuint &Y, const GLuint &W, const GLuint &H, const GLuint &Current, const GLuint &Max) {
    SHADER::pushMatrix();
      SHADER::translate(X,Y,0);
      glColor4fv(HUD_Color_4fv[HUD_Color]);
      glBegin(GL_LINES);
        glVertex2d(-4, H+4);
        glVertex2d(W+4, H+4);
        
        glVertex2d(W+4, H+4);
        glVertex2d(W+4, -4);
        
        glVertex2d(W+4, -4);
        glVertex2d(-4, -4);
      glEnd();
//     DrawHUDLine(X-4, SCREEN_HEIGHT-8, X+W+4, SCREEN_HEIGHT-8, 2); 
//     DrawHUDLine(X+W+4, SCREEN_HEIGHT-8, X+W+4, SCREEN_HEIGHT-112, 2);  
//     DrawHUDLine(X+W+4, SCREEN_HEIGHT-112, X-4, SCREEN_HEIGHT-112, 2);  
    glBegin(GL_QUADS);
//       SHADER::setColor(1.0f, (GLfloat)Current/Max, 0, 0.8f);
//     (1.0f-(GLfloat)health/1000)+colorhalf((1.0f-(GLfloat)health/1000)),
//           SHADER::setColor(1-(jet->energy/100) + colorhalf(1-jet->energy/100), colorhalf(jet->energy/100)+(GLfloat)jet->energy/100, 0, 0.8);
      glColor3f((1.0f-(GLfloat)Current/Max)+colorhalf((1.0f-(GLfloat)Current/Max)), (GLfloat)Current/Max+colorhalf((GLfloat)Current/Max), 0);

      glVertex2d(0, 0);
      glVertex2d(W, 0);
      glVertex2d(W, (((GLdouble)Current/Max)*H));
      glVertex2d(0, (((GLdouble)Current/Max)*H));
    glEnd();  
    SHADER::popMatrix();
  }

  void Road::RenderHUD() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    SHADER::pushMatrix();
      SetOrtho();
        
        glColor4fv(HUD_Color_4fv[HUD_Color]);
          
        
//         DrawHUDLine(0, 48, SCREEN_WIDTH-320, 48, 3);
//         DrawHUDLine(SCREEN_WIDTH-320, 48, SCREEN_WIDTH-192, 128, 3);
//         DrawHUDLine(SCREEN_WIDTH-192, 128, SCREEN_WIDTH, 128,3);

//         DrawHUDLine(0, SCREEN_HEIGHT-128, 192, SCREEN_HEIGHT-128, 3);
//         DrawHUDLine(192, SCREEN_HEIGHT-128, 320, SCREEN_HEIGHT-48, 3);
//         DrawHUDLine(320, SCREEN_HEIGHT-48, SCREEN_WIDTH, SCREEN_HEIGHT-48, 3);
      
//       SHADER::popMatrix();
// 
//       
//       SHADER::pushMatrix();
        
        m_lblPosX.clear();
        m_lblPosX << "X: " << jet->getX();
        m_lblPosX.draw();

        m_lblPosY.clear();
        m_lblPosY << "Y: " << jet->getY();
        m_lblPosY.draw();

        m_lblPosZ.clear();
        m_lblPosZ << "Z: " << jet->getY();
        m_lblPosZ.draw();

        m_lblShotsDone.clear();
        m_lblShotsDone << "Shots: " << ShotsDone;
        m_lblShotsDone.draw();

        m_lblScore.clear();
        m_lblScore << "SCORE: " << jet->GetScore();
        m_lblScore.draw();

        /*radar*/
        GLfloat matrice[16];
        jet->quaternion.CreateMatrix(matrice);
//         jet->rotation.CreateMatrix(matrice);
          
        SHADER::pushMatrix();
          glEnable(GL_DEPTH_TEST);
          SHADER::translate(SCREEN_WIDTH-80, 80, 0);

          SHADER::multMatrix(matrice);
	  
	  glBegin(GL_LINES);
            glVertex3d(24,24,24);
            glVertex3d(-24,24,24);
            glVertex3d(24,-24,24);
            glVertex3d(-24,-24,24);
            glVertex3d(24,24,-24);
            glVertex3d(-24,24,-24);
            glVertex3d(24,-24,-24);
            glVertex3d(-24,-24,-24);
            
            glVertex3d(24,24,24);
            glVertex3d(24,24,-24);
            glVertex3d(24,-24,24);
            glVertex3d(24,-24,-24);
            glVertex3d(-24,24,24);
            glVertex3d(-24,24,-24);
            glVertex3d(-24,-24,24);
            glVertex3d(-24,-24,-24);
            
            glVertex3d(24,24,24);
            glVertex3d(24,-24,24);
            glVertex3d(24,24,-24);
            glVertex3d(24,-24,-24);
            glVertex3d(-24,24,24);
            glVertex3d(-24,-24,24);
            glVertex3d(-24,24,-24);
            glVertex3d(-24,-24,-24);
          glEnd();
          
	  glColor3f(0.3, 0.3, 1);
          glBegin(GL_LINE_LOOP);
          for (drawing_i=0; drawing_i<Radar->GetSegments(); drawing_i++) {
            glVertex3d(Radar->GetX(drawing_i), Radar->GetY(drawing_i), 0);
          }          
          glEnd();
	  glColor3f(0,1,0);
          glBegin(GL_LINE_LOOP);
          for (drawing_i=0; drawing_i<Radar->GetSegments(); drawing_i++) {
            glVertex3d(Radar->GetX(drawing_i), 0, Radar->GetY(drawing_i));
          }          
          glEnd();
	  glColor3f(1,0,0);
          glBegin(GL_LINE_LOOP);
          for (drawing_i=0; drawing_i<Radar->GetSegments(); drawing_i++) {
            glVertex3d(0, Radar->GetX(drawing_i), Radar->GetY(drawing_i));
          }          
          glEnd();
          for (drawing_i=0; drawing_i<enemies.size(); drawing_i++) {
            enemies.at(drawing_i)->DrawRadarPosition(jet->GetPosition(), 92);
          }
          
          glDisable(GL_DEPTH_TEST);
	  Vertex cursor = jet->GetDirection() * 92;
	  glLineWidth(3);
	  SHADER::setColor(1,1,0,0.9);
	  glBegin(GL_LINES);
	    glVertex3d(0,0,0);
// 	    glVertex3d(0,0,-92);
	    glVertex3d(cursor.x, cursor.y, cursor.z);
	  glEnd();
	  glLineWidth(1);
	  
        SHADER::popMatrix();
        
	glColor4fv(HUD_Color_4fv[HUD_Color]);

        m_lblSpeed.clear();
        m_lblSpeed << "SPEED: " << ( jet->GetSpeed() * 270 );
        m_lblSpeed.draw();

        SHADER::pushMatrix();
        SHADER::setColor( 1, 1, 1, 1 );
            SHADER::translate( 32, SCREEN_HEIGHT / 2, 0 );
            static const Buffer fanRing = getFanRotorRing();
            SHADER::drawBuffer( fanRing );
            SHADER::rotate( speed_anim, Axis::Z );
            static const Buffer fanPetals = getFanRotorPetals();
            SHADER::drawBuffer( fanPetals );
        SHADER::popMatrix();

        DrawCyberRingsMini();
        
        DrawHUDBar(12, 12, 36, 96, jet->energy, 100);
        DrawHUDBar(64, 12, 36, 96, jet->GetHealth(), 100);

        glColor4fv(HUD_Color_4fv[HUD_Color]);
      
        m_lblFps.clear();
        m_lblFps << "FPS done: " << FPS << ", calculated: " << CalculatedFPS;
        m_lblFps.draw();
	
	font_pause_txt->PrintTekst(10, 102, "ENG");
	font_pause_txt->PrintTekst(66, 102, "HP");
	
        SHADER::popMatrix();
        
      
  }
  
  void Road::Render3D() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    SetPerspective(angle+jet->GetSpeed()*6);
//     gluPerspective(angle+jet->GetSpeed()*6, (GLdouble)SCREEN_WIDTH/(GLdouble)SCREEN_HEIGHT, 0.001, 2000);

    GLdouble cX = -jet->getX(), cY = -jet->getY(), cZ = -jet->getZ();
    
//     glMatrixMode(GL_MODELVIEW);
    SHADER::pushMatrix();
      SHADER::translate(0, -0.225, -1);
      SHADER::pushMatrix();
        GLfloat matrice[16];
        jet->rotation.CreateMatrix(matrice);
        SHADER::multMatrix(matrice);
        SHADER::translate(cX,cY,cZ);
        map->Draw(); 
        for (drawing_i=0; drawing_i<enemies.size(); drawing_i++) {
          enemies[drawing_i]->Draw(); 
        }
        glLineWidth(2);
        for (drawing_i=0; drawing_i<bullet.size(); drawing_i++) { 
          bullet[drawing_i]->Draw(); 
        }
        for (drawing_i=0; drawing_i<enemybullet.size(); drawing_i++) { 
          enemybullet[drawing_i]->Draw(); 
        }
        glLineWidth(1);
      SHADER::popMatrix();
      jet->Draw();
    SHADER::popMatrix();
    
  }
 
 
void Road::DrawMainMenu() {
    SHADER::pushMatrix();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    DrawClouds();
    DrawCyberRings();

    SHADER::setOrtho( 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT );
    btnSelectMission.Draw();
    btnExit.Draw();
    btnCustomize.Draw();

    SHADER::popMatrix();
}

void Road::DrawClouds() {
    SHADER::pushMatrix();

    const double wr = 0.5 * SCREEN_WIDTH / max_dimention;
    const double hr = 0.5 * SCREEN_HEIGHT / max_dimention;
    SHADER::setOrtho( -wr, wr, -hr, hr );

    static const uint32_t quad = SHADER::getQuad( -0.5, -0.5, 0.5, 0.5 );
    static const uint32_t quadCoord = SHADER::getQuadTextureCoord( 0, 0, 1, 1 );

    SHADER::setTextureCoord( quadCoord );

    m_menuBackground.use();
    SHADER::draw( GL_TRIANGLES, quad, 6 );

    m_menuBackgroundOverlay.use();
    SHADER::draw( GL_TRIANGLES, quad, 6 );

    m_menuBackgroundStarField.use();
    SHADER::draw( GL_TRIANGLES, quad, 6 );

    SHADER::popMatrix();
}
  
  void Road::WinScreen() {
    GameScreen();
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    SHADER::pushMatrix();
    DrawCyberRings();
    glColor4fv(HUD_Color_4fv[0]);
    glBindTexture(GL_TEXTURE_2D, HUDtex);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2d(0, 0);
      glTexCoord2f(1,0); glVertex2d(SCREEN_WIDTH, 0);
      glTexCoord2f(1,1); glVertex2d(SCREEN_WIDTH, SCREEN_HEIGHT);
      glTexCoord2f(0,1); glVertex2d(0, SCREEN_HEIGHT);
    glEnd(); 
    SHADER::popMatrix();
    
    glColor3f(1,1,1);
    font_big->PrintTekst((SCREEN_WIDTH/2)-font_big->GetTextLength("MISSION SUCCESSFUL")/2, SCREEN_HEIGHT-128, "MISSION SUCCESSFUL");
    snprintf(HUDMESSAGE, sizeof(HUDMESSAGE), "Your score: %d", jet->GetScore());
    font_big->PrintTekst((SCREEN_WIDTH/2)-font_big->GetTextLength(HUDMESSAGE)/2, SCREEN_HEIGHT-128-36, HUDMESSAGE);
    btnReturnToMissionSelection.Draw();
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
  
  void Road::DeadScreen() {
    GameScreen();
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    SHADER::pushMatrix();
    DrawCyberRings();
    glColor4fv(HUD_Color_4fv[2]);
    glBindTexture(GL_TEXTURE_2D, HUDtex);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex2d(0, 0);
      glTexCoord2f(1,0); glVertex2d(SCREEN_WIDTH, 0);
      glTexCoord2f(1,1); glVertex2d(SCREEN_WIDTH, SCREEN_HEIGHT);
      glTexCoord2f(0,1); glVertex2d(0, SCREEN_HEIGHT);
    glEnd(); 
    SHADER::popMatrix();
    
    glColor3f(1,1,1);
    font_big->PrintTekst((SCREEN_WIDTH/2)-font_big->GetTextLength("MISSION FAILED")/2, SCREEN_HEIGHT-128, "MISSION FAILED");
    snprintf(HUDMESSAGE, sizeof(HUDMESSAGE), "Your score: %d", jet->GetScore());
    font_big->PrintTekst((SCREEN_WIDTH/2)-font_big->GetTextLength(HUDMESSAGE)/2, SCREEN_HEIGHT-128-36, HUDMESSAGE);
    btnReturnToMissionSelection.Draw();
    
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    
  }
  
void Road::MissionSelectionScreen()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    const double wr = 0.5 * SCREEN_WIDTH / max_dimention;
    const double hr = 0.5 * SCREEN_HEIGHT / max_dimention;
    SHADER::pushMatrix();
        SHADER::setOrtho( -wr, wr, -hr, hr );
        if ( m_currentMap != m_maps.end() ) {
            m_currentMap->drawPreview();
        }
    SHADER::popMatrix();

#if 0
	  snprintf(HUDMESSAGE, sizeof(HUDMESSAGE), "Map: %s", maps_container.at(current_map).name.c_str());
	  font_pause_txt->PrintTekst(SCREEN_WIDTH/2-font_pause_txt->GetTextLength(HUDMESSAGE)/2, SCREEN_HEIGHT-128, HUDMESSAGE);
	  snprintf(HUDMESSAGE, sizeof(HUDMESSAGE), "Enemies: %d", maps_container.at(current_map).enemies);
	  font_pause_txt->PrintTekst(SCREEN_WIDTH/2-font_pause_txt->GetTextLength(HUDMESSAGE)/2, SCREEN_HEIGHT-148, HUDMESSAGE);
	  
#endif
    SHADER::pushMatrix();
        SetOrtho();
        btnStartMission.Draw();
	btnReturnToMainMenu.Draw();
	btnNextMap.Draw();
	btnPrevMap.Draw();
    SHADER::popMatrix();
  }

void Road::GameScreenBriefing()
{
    // draw 3d environment
    GameScreen();

    SHADER::pushMatrix();
        DrawCyberRings();
        drawHudGlow();
        SetOrtho();
        font_pause_txt->PrintTekst(192,SCREEN_HEIGHT-292, "Movement: AWSD QE");
        font_pause_txt->PrintTekst(192,SCREEN_HEIGHT-310, "Speed controll: UO");
        font_pause_txt->PrintTekst(192,SCREEN_HEIGHT-328, "Weapons: JKL");
        font_pause_txt->PrintTekst(192,SCREEN_HEIGHT-346, "Targeting: I");
        font_pause_txt->PrintTekst(192,SCREEN_HEIGHT-380, "Press space to launch...");
        btnGO.Draw();
    SHADER::popMatrix();
}


void Road::ScreenCustomize() {
    SHADER::pushMatrix();
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    DrawClouds();
    DrawCyberRings();
    SetOrtho();
    font_big->PrintTekst( SCREEN_WIDTH / 2 - font_big->GetTextLength( jets_container.at( current_jet ).name.c_str() ) / 2,
                          SCREEN_HEIGHT - 64, jets_container.at( current_jet ).name.c_str() );

    SHADER::pushMatrix();
        SetPerspective( angle );
        SHADER::translate( 0, -0.1, -1.25 );
        SHADER::rotate( 15, Axis::X );
        SHADER::rotate( model_rotation, Axis::Y );
        preview_model.Draw();
    SHADER::popMatrix();
//     SetOrtho();
    btnNextJet.Draw();
    btnPrevJet.Draw();
    btnCustomizeReturn.Draw();
    btnWeap1.Draw();
    btnWeap2.Draw();
    btnWeap3.Draw();
    SHADER::popMatrix();
}
 
 
  void Road::DrawBullets() {
  }
 
