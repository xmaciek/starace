#include "Road.h"

using namespace std;


const Uint32 Road::Time_Interval = (1000*DELTATIME);

Road::Road() {
    
    cout<<Time_Interval<<"\n";
    Display = NULL;
    
    FULLSCREEN = false;
    SCREEN_WIDTH = 960;
    SCREEN_HEIGHT = 540;
    SCREEN_DEPTH = 32;
  
    laser = blaster = torpedo = NULL;
    
    current_filtering = FILTERING_TRILINEAR;
    

    play_sound = true;
    
    Running = true;
    
    doUpdate = true;
    WaitForEnd = true;
    nextTick = 0;
    nextFrame = 0;
    
    Weap1 = Weap2 = Weap3 = 0;
    
    Radar = new Circle(48, 64);
    Rotation = 0;
//     angle = 36;
//     angle = 60;
    angle = 55;
//     angle = 27;
    DynamicCamera = true;
    FramesDone = 0;
    ShotsDone = 0;
    CalculatedFPS = 0;
    tempFPS = 0;
    FPS = 0;
    
    speed_anim = 0;
    
    HUD_Color_4fv[0][0] = 0.0275f;
    HUD_Color_4fv[0][1] = 1.0f;
    HUD_Color_4fv[0][2] = 0.075f;
    HUD_Color_4fv[0][3] = 1.0f;
    
    HUD_Color_4fv[1][0] = 1;
    HUD_Color_4fv[1][1] = 0.6f;
    HUD_Color_4fv[1][2] = 0.1f;
    HUD_Color_4fv[1][3] = 0.8f;
    
    HUD_Color_4fv[2][0] = 1;
    HUD_Color_4fv[2][1] = 0.1f;
    HUD_Color_4fv[2][2] = 0.1f;
    HUD_Color_4fv[2][3] = 1.0f;
    
    HUD_Color = 0;
    
    speed_fan_ring = NULL;
    
    
    ChangeScreen(SA_MAINMENU);
    
    enemies.reserve(100);
    Egarbage.reserve(32);
    bullet.reserve(500);
    enemybullet.reserve(1000);
    Bgarbage.reserve(200);
  }
  
  
  Road::~Road() {
    doUpdate = false;
    
    
    ClearMapData();
    btnExit.SetFont(NULL);
    btnQuitMission.SetFont(NULL);
    btnChangeFiltering.SetFont(NULL);
    btnSelectMission.SetFont(NULL);
    btnGO.SetFont(NULL);
    btnStartMission.SetFont(NULL);
    btnReturnToMainMenu.SetFont(NULL);
    btnReturnToMissionSelection.SetFont(NULL);
    btnNextMap.SetFont(NULL);
    btnPrevMap.SetFont(NULL);
    btnCustomize.SetFont(NULL);
    btnCustomizeReturn.SetFont(NULL);
    btnNextJet.SetFont(NULL);
    btnPrevJet.SetFont(NULL);
    
    if (font_pause_txt!=NULL) { delete font_pause_txt; font_pause_txt=NULL; }
    if (font_gui_txt!=NULL) { delete font_gui_txt; font_gui_txt=NULL; }
    if (font_big!=NULL) { delete font_big; font_big=NULL; }

    
    if (speed_fan_ring !=NULL) { delete speed_fan_ring; }

    if (Radar!=NULL) { delete Radar; }
    glDeleteTextures(1, &HUDtex);
    glDeleteTextures(1, &ButtonTexture);
    for (GLuint i=0; i<maps_container.size(); i++) {
      glDeleteTextures(1, &maps_container.at(i).preview_image);
    }
    cout<<"HERE OK\n"<<"Exiting game should be successful.\n";
  }

GLint Road::OnExecute() {
  if (OnInit()==false) { return -1; }
  SDL_Event Event;
//   int (*fptr)(void*) = (int(*)(void*))&(Road::OnUpdate); // ((Road*)this)->Road::SCREENSARRAY
  thread = SDL_CreateThread(OnUpdateStatic, this);
  if (thread!=NULL) {
    while (Running) {
      while (SDL_PollEvent(&Event)) { OnEvent(Event); }
      OnRender();
    }
    cout<<"Waiting for update thread... ";
    SDL_WaitThread(thread, NULL);
    cout<<"done.\n";
  }
  else { cout<<"-= Unable to start Update thread, terminating! =-\n"<<SDL_GetError()<<"\n"; }
  OnCleanup();
  return 0;
}

void Road::OnEvent(SDL_Event &Event) {
  switch (Event.type) {
    case SDL_QUIT: 
      Running = false; 
      break;
      
    case SDL_KEYDOWN: 
      OnKeyDown(Event.key.keysym.sym, Event.key.keysym.mod, Event.key.keysym.unicode);
      break;
      
    case SDL_KEYUP: 
      OnKeyUp(Event.key.keysym.sym, Event.key.keysym.mod, Event.key.keysym.unicode);
      break;
      
    case SDL_VIDEORESIZE:
      SCREEN_WIDTH = Event.resize.w;
      SCREEN_HEIGHT = Event.resize.h;
      InitNewSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, FULLSCREEN);
      OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (Event.button.button==SDL_BUTTON_LEFT) { OnMouseClickLeft(Event.button.x, Event.button.y); }
      break;
  }
}

void Road::OnCleanup() { 
//   Mix_HaltMusic();
  Mix_HaltChannel(-1);
  
  cout<<Mix_GetError()<<"\n";
  Mix_FreeChunk(laser); laser = NULL;
  Mix_FreeChunk(blaster); blaster = NULL;
  Mix_FreeChunk(torpedo); torpedo = NULL;
  Mix_FreeChunk(click); click = NULL;
  Mix_CloseAudio();
  
  Mix_Quit();
  
    cout<<"HERE OK\n"<<Mix_GetError()<<"\n";
  
  SDL_FreeSurface(Display);
  SDL_Quit(); 
  SaveConfig();
  
}

bool Road::OnInit() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO)<0) { 
    cout<<"Unable to init SDL\n"<<SDL_GetError()<<"\n";
    return false; }
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
 
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
 
  SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 0);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);
 
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
  
//   GLint audio_rate = 44100;
//   Uint16 audio_format = AUDIO_S16SYS;
//   GLint audio_channels = 2;
//   GLint audio_buffers = 4096;
//  Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)
  if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 4096) != 0) {
	fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
	return false;
  }
  
  laser = Mix_LoadWAV("sounds/laser.wav");
  blaster = Mix_LoadWAV("sounds/blaster.wav");
  torpedo = Mix_LoadWAV("sounds/torpedo.wav");
  click = Mix_LoadWAV("sounds/click.wav");
      
  LoadConfig();
    
  setTextureFiltering(current_filtering);

  if (!InitNewSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, FULLSCREEN)) {
    return false;
  }
//   return false;
  SHADER::init();
  InitRoadAdditionsGL();
  OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);
  return true;
}

void Road::OnResize(GLint w, GLint h) {
//   SCREEN_WIDTH = w;
//   SCREEN_HEIGHT = h;
  
  if (w>h) { max_dimention = w; min_dimention = h; }
  else { max_dimention = h; min_dimention = w; }
  
  glViewport(0, 0, w, h);
  btnExit.UpdateCoord((SCREEN_WIDTH/2)+4, SCREEN_HEIGHT*0.15);
  btnQuitMission.UpdateCoord((SCREEN_WIDTH/2)-196, SCREEN_HEIGHT*0.15);
  btnChangeFiltering.UpdateCoord(512, SCREEN_HEIGHT-192);
  btnSelectMission.UpdateCoord((SCREEN_WIDTH/2)-96, SCREEN_HEIGHT*0.15+52);
  btnGO.UpdateCoord((SCREEN_WIDTH/2)-96, SCREEN_HEIGHT*0.15);
  btnStartMission.UpdateCoord((SCREEN_WIDTH/2)+4, SCREEN_HEIGHT*0.15);
  btnReturnToMainMenu.UpdateCoord((SCREEN_WIDTH/2)-196, SCREEN_HEIGHT*0.15);
  btnReturnToMissionSelection.UpdateCoord((SCREEN_WIDTH/2)-96, SCREEN_HEIGHT*0.15);
  btnNextMap.UpdateCoord(SCREEN_WIDTH-240, SCREEN_HEIGHT/2-24);
  btnPrevMap.UpdateCoord(48, SCREEN_HEIGHT/2-24);
  btnCustomize.UpdateCoord((SCREEN_WIDTH/2)-196, SCREEN_HEIGHT*0.15);
  btnCustomizeReturn.UpdateCoord((SCREEN_WIDTH/2)-96, SCREEN_HEIGHT*0.15+52);
  btnNextJet.UpdateCoord(SCREEN_WIDTH-240, SCREEN_HEIGHT/2-24);
  btnPrevJet.UpdateCoord(48, SCREEN_HEIGHT/2-24);
  
  btnWeap1.UpdateCoord(SCREEN_WIDTH/2-196-96, SCREEN_HEIGHT*0.15+52-76);
  btnWeap2.UpdateCoord(SCREEN_WIDTH/2-96, SCREEN_HEIGHT*0.15+52-76);
  btnWeap3.UpdateCoord(SCREEN_WIDTH/2+100, SCREEN_HEIGHT*0.15+52-76);
  
}




  void Road::InitRoadAdditionsGL() {
  
//   init functons
    
     if( TTF_Init()<0) { cout<<"Unable to initialize library: "<<TTF_GetError()<<"\n";}
//     setTextureFiltering(FILTERING_ANISOTROPIC_X16);
    font_pause_txt = new Font("misc/DejaVuSans-Bold.ttf", 18);
    font_gui_txt = new Font("misc/DejaVuSans-Bold.ttf", 12);
    font_big = new Font("misc/DejaVuSans-Bold.ttf", 32);
    TTF_Quit(); 
    
    ButtonTexture = LoadTexture("textures/button1.tga");
    
    btnChangeFiltering.SetFont(font_gui_txt);
    btnChangeFiltering.SetText("Anisotropic x16");
    btnChangeFiltering.SetTexture(ButtonTexture);
    
    btnExit.SetFont(font_gui_txt);
    btnExit.SetText("Exit Game");
    btnExit.SetTexture(ButtonTexture);
    
    btnSelectMission.SetFont(font_gui_txt);
    btnSelectMission.SetText("Select Mission");
    btnSelectMission.SetTexture(ButtonTexture);
    
    btnQuitMission.SetFont(font_gui_txt);
    btnQuitMission.SetText("Quit Mission");
    btnQuitMission.SetTexture(ButtonTexture);
    
    btnStartMission.SetFont(font_gui_txt);
    btnStartMission.SetText("Start Mission");
    btnStartMission.SetTexture(ButtonTexture);
    
    btnReturnToMissionSelection.SetFont(font_gui_txt);
    btnReturnToMissionSelection.SetText("Return");
    btnReturnToMissionSelection.SetTexture(ButtonTexture);
    
    btnReturnToMainMenu.SetFont(font_gui_txt);
    btnReturnToMainMenu.SetText("Return");
    btnReturnToMainMenu.SetTexture(ButtonTexture);
    
    btnGO.SetFont(font_gui_txt);
    btnGO.SetText("GO!");
    btnGO.SetTexture(ButtonTexture);
    
    btnNextMap.SetFont(font_gui_txt);
    btnNextMap.SetText("Next Map");
    btnNextMap.SetTexture(ButtonTexture);
    
    btnPrevMap.SetFont(font_gui_txt);
    btnPrevMap.SetText("Previous Map");
    btnPrevMap.SetTexture(ButtonTexture);
    
    btnNextJet.SetFont(font_gui_txt);
    btnNextJet.SetText("Next Jet");
    btnNextJet.SetTexture(ButtonTexture);
    
    btnPrevJet.SetFont(font_gui_txt);
    btnPrevJet.SetText("Previous Jet");
    btnPrevJet.SetTexture(ButtonTexture);
    
    btnCustomizeReturn.SetFont(font_gui_txt);
    btnCustomizeReturn.SetText("Done");
    btnCustomizeReturn.SetTexture(ButtonTexture);
    
    btnCustomize.SetFont(font_gui_txt);
    btnCustomize.SetText("Customise");
    btnCustomize.SetTexture(ButtonTexture);
    
    btnWeap1.SetFont(font_gui_txt);
    btnWeap1.SetTexture(ButtonTexture);
    
    btnWeap2.SetFont(font_gui_txt);
    btnWeap2.SetTexture(ButtonTexture);
    
    btnWeap3.SetFont(font_gui_txt);
    btnWeap3.SetTexture(ButtonTexture);
    
    switch (Weap1) {
      case 0: btnWeap1.SetText("Laser"); break;
      case 1: btnWeap1.SetText("Blaster"); break;
      case 2: btnWeap1.SetText("Torpedo"); break;
    }
    switch (Weap2) {
      case 0: btnWeap2.SetText("Laser"); break;
      case 1: btnWeap2.SetText("Blaster"); break;
      case 2: btnWeap2.SetText("Torpedo"); break;
    }
    switch (Weap3) {
      case 0: btnWeap3.SetText("Laser"); break;
      case 1: btnWeap3.SetText("Blaster"); break;
      case 2: btnWeap3.SetText("Torpedo"); break;
    }
    
    
    TimePassed = time(NULL);
    HUDtex = LoadTexture("textures/HUDtex.tga");

    m_menuBackground.load( "textures/background.tga" );
    m_menuBackgroundOverlay.load( "textures/background-overlay.tga" );
    m_menuBackgroundStarField.load( "textures/star_field_transparent.tga" );

    speed_fan_ring = new Circle(32, 26);

    m_ringTextureA.load( "textures/cyber_ring1.tga" );
    m_ringTextureB.load( "textures/cyber_ring2.tga" );
    m_ringTextureC.load( "textures/cyber_ring3.tga" );
    cyber_ring_rotation[0] = 0;
    cyber_ring_rotation[1] = 0;
    cyber_ring_rotation[2] = 0;
    cyber_ring_rotation[3] = 0;
    cyber_ring_rotation_direction[0] = false;
    cyber_ring_rotation_direction[1] = false;
    cyber_ring_rotation_direction[2] = false;
    cyber_ring_rotation_direction[3] = false;
    
    
    GLfloat temp_colors[4][4] = { {1,1,1,0.8}, {1,1,1,0.7}, {1,1,1,0.6}, {1,1,1,0.7}}; 
    
    memcpy(cyber_ring_color, temp_colors, sizeof(GLfloat)*16);
    
    
    glEnable(GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
    LightAmbient[0] = 0.5f;
    LightAmbient[1] = 0.5f;
    LightAmbient[2] = 0.5f;
    LightAmbient[3] = 1.0f;
    
    LightDiffuse[0] = 0.8f;
    LightDiffuse[1] = 0.8f;
    LightDiffuse[2] = 0.8f;
    LightDiffuse[3] = 1.0f;
    
    LightPosition[0] = 0;
    LightPosition[1] = 1;
    LightPosition[2] = 1;
    LightPosition[3] = 1;
  
    
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glMaterialfv(GL_FRONT, GL_AMBIENT, LightAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    glEnable(GL_LIGHT0);
  
  
    GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};
    glFogi(GL_FOG_MODE, GL_LINEAR);    
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.1f);  
    glHint(GL_FOG_HINT, GL_NICEST);
    glFogf(GL_FOG_START, 10.0f);             
    glFogf(GL_FOG_END, 25.0f);  
    
    
    
  
  GLfloat tempcolor[4][4] = {
    {0.3,0.8,1,1}, {1,0.8,0.1,1},
     {1,0.3,0.8,1}, {1,1,0,1} };
  BulletProto tmpWeapon;
  tmpWeapon.type = Bullet::SLUG;
  tmpWeapon.delay = 0.1;
  tmpWeapon.energy = 15;
  tmpWeapon.damage = 1;
  tmpWeapon.score_per_hit = 1;
  memcpy(tmpWeapon.color1, tempcolor[3], 4*sizeof(GLfloat));
  memcpy(tmpWeapon.color2, tempcolor[1], 4*sizeof(GLfloat));
  Weapons[0] = tmpWeapon;
  
  tmpWeapon.type = Bullet::BLASTER;
  tmpWeapon.speed = 16;
  tmpWeapon.damage = 10;
  tmpWeapon.energy = 10;
  tmpWeapon.delay = 0.2;
  memcpy(tmpWeapon.color1, tempcolor[0], 4*sizeof(GLfloat));
  tmpWeapon.score_per_hit = 30;
  Weapons[1] = tmpWeapon;
  
  memcpy(tmpWeapon.color1, tempcolor[1], 4*sizeof(GLfloat));
  tmpWeapon.delay = 0.4;
  Weapons[3] = tmpWeapon;
  
  tmpWeapon.type = Bullet::TORPEDO;
  tmpWeapon.damage = 1;
  tmpWeapon.delay = 0.4;
  tmpWeapon.energy = 1;
  tmpWeapon.speed = 8;
  tmpWeapon.score_per_hit = 2;
  memcpy(tmpWeapon.color1, tempcolor[2], 4*sizeof(GLfloat));
  Weapons[2] = tmpWeapon;
    
    
    
  }
  

  void Road::UpdateCyberRings() {
    cyber_ring_rotation[0] += 25.0 * DELTATIME;
    cyber_ring_rotation[1] -= 15.0 * DELTATIME;
    if (cyber_ring_rotation_direction[2]) { 
      cyber_ring_rotation[2] += 35.0 * DELTATIME;
    } else { cyber_ring_rotation[2] -= 20.0 * DELTATIME; }
//     cyber_ring_rotation[3] += 20.0 * DELTATIME;
    
    if (cyber_ring_rotation[0]>=360) { cyber_ring_rotation[0] -= 360; }
    if (cyber_ring_rotation[1]<0) { cyber_ring_rotation[1] += 360; }
    if (cyber_ring_rotation[2]<0) { cyber_ring_rotation_direction[2] = true; }
    if (cyber_ring_rotation[2]>90) { cyber_ring_rotation_direction[2] = false; }
//     if (cyber_ring_rotation[3]>=360) { cyber_ring_rotation[3] -= 360; }
    
    
  }
  
  
  
  void Road::OnRender() {
    timeS = SDL_GetTicks();
    switch (SCREEN) {
      case SA_GAMESCREEN: GameScreen(); break;
      case SA_GAMESCREEN_PAUSED: GameScreenPaused(); break;
      case SA_GAMESCREEN_BRIEFING: GameScreenBriefing(); break;
      case SA_DEADSCREEN: DeadScreen(); break;
      case SA_WINSCREEN: WinScreen(); break;
      case SA_MISSIONSELECTION: MissionSelectionScreen(); break;
      case SA_MAINMENU: DrawMainMenu(); break;
      case SA_CUSTOMIZE: ScreenCustomize(); break;
      default: break;
    }
    SDL_GL_SwapBuffers(); 
  }

  void Road::OnUpdate() {
    while (Running){
      switch (SCREEN) {
        case SA_GAMESCREEN: GameUpdate(); break;
        case SA_GAMESCREEN_PAUSED: GameUpdatePaused(); break;
	case SA_GAMESCREEN_BRIEFING: GameScreenBriefingUpdate(); break;
	case SA_DEADSCREEN: DeadScreenUpdate(); break;
	case SA_WINSCREEN: WinUpdate(); break;
	case SA_MISSIONSELECTION: MissionSelectionUpdate(); break;
	case SA_MAINMENU: UpdateMainMenu(); break;
	case SA_CUSTOMIZE: UpdateCustomize(); break;
      }
      SDL_Delay(Delay());
    }
  }
  
  int Road::OnUpdateStatic(void *param) {
    ((Road*)param)->OnUpdate();
    return 0;
  }

  Uint32 Road::Delay() {
    static Uint32 next = 0;
    Uint32 now = SDL_GetTicks();
    if (next<=now) {
      next = now + Time_Interval;
      return 0;
    }
    return next-now;
  }
  
  void Road::Pause() {
    ChangeScreen(SA_GAMESCREEN_PAUSED);
  }
  
  void Road::Unpause() {
    ChangeScreen(SA_GAMESCREEN);
  }  
    
  void Road::GameUpdatePaused() {
    UpdateCyberRings();
  }
  
  
  void Road::GameUpdate() {
    if (jet->GetStatus()==SAObject::DEAD) { ChangeScreen(SA_DEADSCREEN); }
    if (enemies.size()==0) { ChangeScreen(SA_WINSCREEN); }
    if (jet->GetHealth()<=20) { HUD_Color = 2; }
    else { HUD_Color = 0; }
    
    if (jet->IsShooting(0)) { AddBullet(0); }
    if (jet->IsShooting(1)) { AddBullet(1); }
    if (jet->IsShooting(2)) { AddBullet(2); }
    
    for (update_i=0; update_i<enemies.size(); update_i++) {
      enemies.at(update_i)->Update();
      if (enemies.at(update_i)->IsWeaponReady()) {
        enemybullet.push_back(enemies.at(update_i)->GetWeapon());
      }
      if (enemies.at(update_i)->GetStatus()==Enemy::DEAD) { 
//         cout<<"Moving Enemy to Garbage.\n";
	jet->AddScore(enemies.at(update_i)->GetScore(), true);
        Egarbage.push_back(enemies.at(update_i));
        enemies.erase(enemies.begin()+update_i);
//         update_i-=1;
      }
    }
    

    for (update_i=0; update_i<enemybullet.size(); update_i++) {
      enemybullet.at(update_i)->ProcessCollision(*jet);
    }
    
    jet->Update();
    speed_anim += jet->GetSpeed()*(270.0*DELTATIME);
    if (speed_anim>=360) { speed_anim -= 360; }
    map->GetJetData(jet->GetPosition(), jet->GetVelocity());
    map->Update();
    
    for (update_i=0; update_i<bullet.size(); update_i++) { 
      
      for (update_i2=0; update_i2<enemies.size(); update_i2++) {
	bullet.at(update_i)->ProcessCollision(*enemies.at(update_i2));
      }
      
      bullet.at(update_i)->Update(); 
      if (bullet.at(update_i)->GetStatus()==Bullet::DEAD) { 
//         cout<<"Moving Bullet to Garbage. \n";
        Bgarbage.push_back(bullet.at(update_i));
        bullet.erase(bullet.begin()+update_i);
//         update_i-=1;
      }
    }
    for (update_i=0; update_i<enemybullet.size(); update_i++) { 
      enemybullet.at(update_i)->Update(); 
      if (enemybullet.at(update_i)->GetStatus()==Bullet::DEAD) { 
//         cout<<"Moving Bullet to Garbage. \n";
        Bgarbage.push_back(enemybullet.at(update_i));
        enemybullet.erase(enemybullet.begin()+update_i);
//         update_i-=1;
      }
    }
    

    for (update_i=0; update_i<Egarbage.size(); update_i++) {
      if (Egarbage.at(update_i)->DeleteMe()) {
//         cout<<"Enemy garbage size: "<<Egarbage.size()<<", Deleting from garbage: ";
        delete Egarbage.at(update_i);
        Egarbage.erase(Egarbage.begin()+update_i);
//         update_i-=1;
      }
    }
    for (update_i=0; update_i<Bgarbage.size(); update_i++) {
      if (Bgarbage.at(update_i)->DeleteMe()) {
//         cout<<"Bullet garbage size: "<<Bgarbage.size()<<", Deleting from garbage: ";
        delete Bgarbage.at(update_i);
        Bgarbage.erase(Bgarbage.begin()+update_i);
//         update_i-=1;
      }
    }
    UpdateCyberRings();
  }
  
  





  void Road::AddBullet(GLuint wID) {
    if (jet->IsWeaponReady(wID)) {
      jet->TakeEnergy(wID);
      bullet.push_back( jet->GetWeaponType(wID) );
      ShotsDone++;
      if (bullet.at(bullet.size()-1)->GetType()==Bullet::BLASTER) {
	PlaySound(blaster); }
      if (bullet.at(bullet.size()-1)->GetType()==Bullet::SLUG) {
	PlaySound(laser); }
      if (bullet.at(bullet.size()-1)->GetType()==Bullet::TORPEDO) {
	PlaySound(torpedo); }
    }
  }

  


  void Road::OnMouseClickLeft(GLint X, GLint Y) {
//     cout<<"X: "<<X<<" Y: "<<Y<<"\n";
//     X = SCREEN_WIDTH - X;
    Y = SCREEN_HEIGHT - Y;
    switch (SCREEN) {
      case SA_GAMESCREEN_PAUSED:
        if (btnQuitMission.IsClicked(X,Y)) { 
          PlaySound(click);
          ChangeScreen(SA_DEADSCREEN); 
          break;
          }
//         if (btnChangeFiltering.IsClicked(X,Y)) {
//           PlaySound(click);
//           current_filtering++;
//           if (current_filtering>7) { current_filtering = FILTERING_TRILINEAR; }
//           setAllTexturesFiltering(current_filtering);
//           switch (current_filtering) {
//             case FILTERING_TRILINEAR: btnChangeFiltering.SetText("Trilinear"); break;
//             case FILTERING_ANISOTROPIC_X2: btnChangeFiltering.SetText("Anisotropic x2"); break;
//             case FILTERING_ANISOTROPIC_X4: btnChangeFiltering.SetText("Anisotropic x4"); break;
//             case FILTERING_ANISOTROPIC_X8: btnChangeFiltering.SetText("Anisotropic x8"); break;
//             case FILTERING_ANISOTROPIC_X16: btnChangeFiltering.SetText("Anisotropic x16"); break;
// 	    default: btnChangeFiltering.SetText("Trilinear");  current_filtering = FILTERING_TRILINEAR; break;
//           }
//           break;
//           
//         } break;
        
      case SA_MAINMENU:
        if (btnSelectMission.IsClicked(X, Y)) { 
	  PlaySound(click);
	  ChangeScreen(SA_MISSIONSELECTION);  
	  break; }
        if (btnExit.IsClicked(X, Y)) {  
	  PlaySound(click);
	  Running = false; 
	  break; }
        if (btnCustomize.IsClicked(X, Y)) {  
	  PlaySound(click);
	  ChangeScreen(SA_CUSTOMIZE); 
	  break; }
	  break;
      case SA_MISSIONSELECTION:
	if (btnStartMission.IsClicked(X, Y)) {  
	  PlaySound(click);
	  ChangeScreen(SA_GAMESCREEN_BRIEFING); 
	  break; }
	if (btnReturnToMainMenu.IsClicked(X, Y)) {  
	  PlaySound(click);
	  ChangeScreen(SA_MAINMENU); 
	  break; }
	if (btnNextMap.IsClicked(X, Y)) { 
	  current_map++; 
	  if (current_map==maps_container.size()-1) { btnNextMap.Disable(); }
	  btnPrevMap.Enable();
      PlaySound(click);
	  break; }
	if (btnPrevMap.IsClicked(X, Y)) {
	  current_map--; 
	  if (current_map==0) { btnPrevMap.Disable(); }
	  if (maps_container.size()>1) { btnNextMap.Enable(); }
      PlaySound(click);
	  break; }
	  break;
      case SA_DEADSCREEN:
	if (btnReturnToMissionSelection.IsClicked(X, Y)) {  
	  PlaySound(click);
	  ChangeScreen(SA_MISSIONSELECTION); 
	  break; }
      case SA_WINSCREEN:
	if (btnReturnToMissionSelection.IsClicked(X, Y)) {  
	  PlaySound(click);
	  ChangeScreen(SA_MISSIONSELECTION); 
	  break; }
	  break;
      case SA_GAMESCREEN_BRIEFING:
	if (btnGO.IsClicked(X, Y)) {  
	  PlaySound(click);
	  ChangeScreen(SA_GAMESCREEN); 
	  break; }
	  break;
      case SA_CUSTOMIZE:
	if (btnNextJet.IsClicked(X, Y)) { 
	  PlaySound(click);
	  current_jet++;
	  if (current_jet==jets_container.size()-1) { btnNextJet.Disable(); }
	  btnPrevJet.Enable();
	  preview_model.Load_OBJ(jets_container.at(current_jet).model_file.c_str());
	  preview_model.CalculateNormal();
	  preview_model.setTexture( jets_container.at( current_jet ).model_texture );
	  break;
	}
	if (btnPrevJet.IsClicked(X, Y)) { 
	  PlaySound(click);
	  current_jet--;
	  if (current_jet==0) { btnPrevJet.Disable(); }
	  if (jets_container.size()>1) { btnNextJet.Enable(); }
	  preview_model.Load_OBJ(jets_container.at(current_jet).model_file.c_str());
	  preview_model.CalculateNormal();
	  preview_model.setTexture( jets_container.at( current_jet ).model_texture );
	  break;
	}
	if (btnCustomizeReturn.IsClicked(X, Y)) {
	  PlaySound(click);
	  ChangeScreen(SA_MAINMENU);
	  break;
	}
	if (btnWeap1.IsClicked(X, Y)) { 
	  PlaySound(click);
	  Weap1++;
	  if (Weap1==3) { Weap1=0; }
	  if (Weap1==0) { btnWeap1.SetText("Laser"); }
	  if (Weap1==1) { btnWeap1.SetText("Blaster"); }
	  if (Weap1==2) { btnWeap1.SetText("Torpedo"); }
	  break;
	}
	if (btnWeap2.IsClicked(X, Y)) { 
	  PlaySound(click);
	  Weap2++;
	  if (Weap2==3) { Weap2=0; }
	  if (Weap2==0) { btnWeap2.SetText("Laser"); }
	  if (Weap2==1) { btnWeap2.SetText("Blaster"); }
	  if (Weap2==2) { btnWeap2.SetText("Torpedo"); }
	  break;
	}
	if (btnWeap3.IsClicked(X, Y)) { 
	  PlaySound(click);
	  Weap3++;
	  if (Weap3==3) { Weap3=0; }
	  if (Weap3==0) { btnWeap3.SetText("Laser"); }
	  if (Weap3==1) { btnWeap3.SetText("Blaster"); }
	  if (Weap3==2) { btnWeap3.SetText("Torpedo"); }
	  break;
	}
	  break;
      default: break;
    }
  }

  void Road::Retarget() {
    if (enemies.size()==0) { return; }
    jet->LockTarget(enemies[rand()%enemies.size()]);
    
  }

void Road::UpdateMainMenu()
{
    UpdateCyberRings();
}

  
  void Road::GameScreenBriefingUpdate() {
    GameUpdatePaused();
  }
  
  
  void Road::ClearMapData() {
      
    cout<<"Moving all enemies to garbage.\n";
    for (GLuint i=0; i<enemies.size(); i++) {
      Egarbage.push_back(enemies.at(i));
    }
    enemies.clear();
    cout<<"Moving all bullets to garbage.\n";
    for (GLuint i=0; i<bullet.size(); i++) {
      Bgarbage.push_back(bullet.at(i));
    }
    bullet.clear();    
    for (GLuint i=0; i<enemybullet.size(); i++) {
      Bgarbage.push_back(enemybullet.at(i));
    }
    enemybullet.clear();
    
    cout<<"Cleaning garbage...\n";
    for (GLuint i=0; i<Egarbage.size(); i++) {
      delete Egarbage.at(i);
    }
    Egarbage.clear();
    
    for (GLuint i=0; i<Bgarbage.size(); i++) {
      delete Bgarbage.at(i);
    }
    Bgarbage.clear();
    cout<<"Cleaning garbage: done.\n";
    if (map!=NULL) { delete map; map = NULL; }
    if (jet!=NULL) { delete jet; jet = NULL; }
    
  }
  
  
  
  
  void Road::CreateMapData(MapProto map_data, ModelProto model_data) {
    ShotsDone = 0;
    HUD_Color = 0;
    jet = new Jet(model_data);
    map = new Map(map_data);
    jet->SetWeapon(Weapons[Weap1], 0);
    jet->SetWeapon(Weapons[Weap2], 1);
    jet->SetWeapon(Weapons[Weap3], 2);    

    for (GLuint i=0; i<map_data.enemies; i++) {
      enemies.push_back(new Enemy());
      enemies.at(i)->SetTarget(jet);
      enemies.at(i)->SetWeapon(Weapons[3]);
    }
    for (GLuint i=0; i<maps_container.size(); i++) {
      glDeleteTextures(1, &maps_container.at(i).preview_image);
      maps_container.at(i).preview_image = 0;
    }
    
  }
  
void Road::MissionSelectionUpdate() {
  UpdateCyberRings();
  
}  
  
  
void Road::ChangeScreen(GLubyte SCR) {
  if (SCR==SA_GAMESCREEN) { SDL_ShowCursor(0); }
    else { SDL_ShowCursor(1); }
  switch (SCR) {
    case SA_GAMESCREEN: SCREEN = SCR; break;
    case SA_GAMESCREEN_PAUSED: SCREEN = SCR; break;
    case SA_GAMESCREEN_BRIEFING: 
      CreateMapData(maps_container.at(current_map), jets_container.at(current_jet));
      SCREEN = SCR; 
      break;
    case SA_MISSIONSELECTION:
      ClearMapData();
      SCREEN = SCR;
      break;
    case SA_DEADSCREEN:
      SCREEN = SCR; 
      break;
    case SA_MAINMENU: SCREEN = SCR; break;
    case SA_CUSTOMIZE: 
      model_rotation = 135.0; 
      cout<<current_jet<<" "<<jets_container.size()<<"\n";
      cout<<jets_container.at(current_jet).name.c_str()<<"\n";
      preview_model.Load_OBJ(jets_container.at(current_jet).model_file.c_str());
      preview_model.setTexture( jets_container.at( current_jet ).model_texture );
      preview_model.CalculateNormal();
      SCREEN = SCR; 
      break;
    case SA_WINSCREEN: 
      SCREEN = SCR;
      break;
    default: break;
  }
  
  
}  
  
  

void Road::GoFullscreen(bool &b) {
  b=!b;
  InitNewSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, b);
}


bool Road::InitNewSurface(GLint W, GLint H, GLint D, bool F) {
  SDL_Surface *tmp = Display, *tmp2 = NULL;
  if (F) { tmp2 = SDL_SetVideoMode(W, H, D, SDL_DOUBLEBUF | SDL_OPENGL | SDL_FULLSCREEN); }
  else { tmp2 = SDL_SetVideoMode(W, H, D, SDL_DOUBLEBUF | SDL_OPENGL | SDL_RESIZABLE); }
  if (tmp2==NULL) { 
    cout<<"Unable to create display surface:\n";
    string error(SDL_GetError());
    cout<<error<<"\n";
    return false; }
  Display = tmp2;
  if (tmp!=NULL) { SDL_FreeSurface(tmp); }
  return true;
}
 
void Road::LoadMapProto() {
  cout<<"Loadings maps... ";
  maps_container.clear();
  MapProto map;
  fstream MapFile("maps.cfg", fstream::in);
  char value_1[48], value_2[48];
  string line;
  while (getline(MapFile, line)) {
    sscanf(line.c_str(), "%s %s", value_1, value_2);
    if (strcmp(value_1, "[MAP]")==0) { maps_container.push_back(map); }
    if (strcmp(value_1, "name")==0) { maps_container.at(maps_container.size()-1).name = value_2; }
    if (strcmp(value_1, "enemies")==0) { maps_container.at(maps_container.size()-1).enemies = atoi(value_2); }
    if (strcmp(value_1, "top")==0) { maps_container.at(maps_container.size()-1).TOP = value_2; }
    if (strcmp(value_1, "bottom")==0) { maps_container.at(maps_container.size()-1).BOTTOM = value_2; }
    if (strcmp(value_1, "left")==0) { maps_container.at(maps_container.size()-1).LEFT = value_2; }
    if (strcmp(value_1, "right")==0) { maps_container.at(maps_container.size()-1).RIGHT = value_2; }
    if (strcmp(value_1, "front")==0) { maps_container.at(maps_container.size()-1).FRONT = value_2; }
    if (strcmp(value_1, "back")==0) { maps_container.at(maps_container.size()-1).BACK = value_2; }
    if (strcmp(value_1, "preview")==0) { maps_container.at(maps_container.size()-1).preview_image_location = value_2; }

  } 
  MapFile.close();
  if (maps_container.size()==0) { 
    maps_container.push_back(map); 
    btnNextMap.Disable();
  }
  current_map = 0;
  btnPrevMap.Disable();
  cout<<"done\n";
}

void Road::LoadJetProto() {
  cout<<"Loadings jets... ";
  jets_container.clear();
  ModelProto mod;
  fstream JetFile("jets.cfg", fstream::in);
  char value_1[48], value_2[48];
  string line;
  while (getline(JetFile, line)) {
    sscanf(line.c_str(), "%s %s", value_1, value_2);
//     cout<<value_1<<" "<<value_2<<"\n";
    if (strcmp(value_1, "[JET]")==0) { jets_container.push_back(mod); }
    if (strcmp(value_1, "name")==0) { jets_container.at(jets_container.size()-1).name = value_2; }
    if (strcmp(value_1, "texture")==0) { jets_container.at(jets_container.size()-1).model_texture = value_2; }
    if (strcmp(value_1, "model")==0) { jets_container.at(jets_container.size()-1).model_file = value_2; }
    if (strcmp(value_1, "scale")==0) { jets_container.at(jets_container.size()-1).scale = atof(value_2); }
  } 
  JetFile.close();
  if (jets_container.size()==0) { 
    cout<<"no jets\n";
    jets_container.push_back(mod);
  }
  if (jets_container.size()==1) { 
    btnNextJet.Disable();  }
  cout<<"size "<<jets_container.size()<<"\n";
  current_jet = 0;
  for (GLuint i=0; i<jets_container.size(); i++) {
    if (LastSelectedJetName == jets_container.at(i).name) {
      current_jet = i;
    }
  }
  if (current_jet==0) { btnPrevJet.Disable(); }
  if (current_jet==jets_container.size()-1) { btnNextJet.Disable(); }
 
  
  cout<<"done\n";
}

void Road::LoadConfig() {
  cout<<"Loading from configuration file... ";
  fstream ConfigFile("config.cfg", ios::in);
  char value_1[48], value_2[48];
  string line;
  while (getline(ConfigFile, line)) {
    sscanf(line.c_str(), "%s %s", value_1, value_2);
    if (strcmp(value_1, "width")==0) { SCREEN_WIDTH = atoi(value_2); }
    if (strcmp(value_1, "height")==0) { SCREEN_HEIGHT = atoi(value_2); }
    if (strcmp(value_1, "fullscreen")==0) { FULLSCREEN = (bool)atoi(value_2); }
    if (strcmp(value_1, "texturefiltering")==0) { current_filtering = atoi(value_2); }
    if (strcmp(value_1, "jet")==0) { LastSelectedJetName = value_2; }
    if (strcmp(value_1, "weap1")==0) { 
      if (strcmp(value_2, "laser")==0) {Weap1 = 0;}
      if (strcmp(value_2, "blaster")==0) {Weap1 = 1;}
      if (strcmp(value_2, "torpedo")==0) {Weap1 = 2;}
      
    }
    if (strcmp(value_1, "weap2")==0) { 
      if (strcmp(value_2, "laser")==0) {Weap2 = 0;} 
      if (strcmp(value_2, "blaster")==0) {Weap2 = 1;}
      if (strcmp(value_2, "torpedo")==0) {Weap2 = 2;} 
      
    }
    if (strcmp(value_1, "weap3")==0) { 
      if (strcmp(value_2, "laser")==0) {Weap3 = 0;}
      if (strcmp(value_2, "blaster")==0) {Weap3 = 1;}
      if (strcmp(value_2, "torpedo")==0) {Weap3 = 2;}
      
    }
    if (strcmp(value_1, "sound")==0) { play_sound = (bool)atoi(value_2); }
  }
  ConfigFile.close();
  cout<<"done.\n";
  LoadMapProto();
  LoadJetProto();
}

void Road::SaveConfig() {
  cout<<"Saving to configuration file... ";
  fstream ConfigFile("config.cfg", ios::out);
  ConfigFile << "width " << SCREEN_WIDTH <<"\n";
  ConfigFile << "height " << SCREEN_HEIGHT <<"\n";
  ConfigFile << "fullscreen " << FULLSCREEN <<"\n";
  ConfigFile << "texturefiltering " << current_filtering <<"\n";
  ConfigFile << "sound " << play_sound <<"\n";
  ConfigFile << "jet " << jets_container.at(current_jet).name<<"\n";
  ConfigFile << "weap1 ";
  switch (Weap1) {
    case 0: ConfigFile << "laser\n"; break;
    case 1: ConfigFile << "blaster\n"; break;
    case 2: ConfigFile << "torpedo\n"; break;
  }
  ConfigFile << "weap2 ";
  switch (Weap2) {
    case 0: ConfigFile << "laser\n"; break;
    case 1: ConfigFile << "blaster\n"; break;
    case 2: ConfigFile << "torpedo\n"; break;
  }
  ConfigFile << "weap3 ";
  switch (Weap3) {
    case 0: ConfigFile << "laser\n"; break;
    case 1: ConfigFile << "blaster\n"; break;
    case 2: ConfigFile << "torpedo\n"; break;
  }
  ConfigFile.close();
  cout<<"done.\n";
}

void Road::SetOrtho()
{
  SHADER::setOrtho( 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT );
}

void Road::SetPerspective( double angle )
{
    SHADER::setPerspective( angle, (double)SCREEN_WIDTH / SCREEN_HEIGHT, 0.001, 2000 );
}

void Road::UpdateCustomize() {
  UpdateCyberRings();
  model_rotation += 30.0*DELTATIME;
  if (model_rotation>=360.0) { model_rotation-= 360.0; }
}


void Road::PlaySound(Mix_Chunk *sound) {
  if (play_sound) {
    Mix_Playing(Mix_PlayChannel(-1, sound, 0));
  }
}

void Road::WinUpdate()
{
  UpdateCyberRings();
}

void Road::DeadScreenUpdate() {
  UpdateCyberRings();
}
