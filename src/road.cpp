#include "road.hpp"

#include <algorithm>
#include <random>

const Uint32 Road::Time_Interval = ( 1000 * DELTATIME );

Road::Road()
{
    SCREEN_WIDTH = 960;
    SCREEN_HEIGHT = 540;
    SCREEN_DEPTH = 32;

    play_sound = true;

    Running = true;

    WaitForEnd = true;
    m_radar = new Circle( 48, 64 );
    angle = 55;
    DynamicCamera = true;

    HUD_Color_4fv[ 0 ][ 0 ] = 0.0275f;
    HUD_Color_4fv[ 0 ][ 1 ] = 1.0f;
    HUD_Color_4fv[ 0 ][ 2 ] = 0.075f;
    HUD_Color_4fv[ 0 ][ 3 ] = 1.0f;

    HUD_Color_4fv[ 1 ][ 0 ] = 1;
    HUD_Color_4fv[ 1 ][ 1 ] = 0.6f;
    HUD_Color_4fv[ 1 ][ 2 ] = 0.1f;
    HUD_Color_4fv[ 1 ][ 3 ] = 0.8f;

    HUD_Color_4fv[ 2 ][ 0 ] = 1;
    HUD_Color_4fv[ 2 ][ 1 ] = 0.1f;
    HUD_Color_4fv[ 2 ][ 2 ] = 0.1f;
    HUD_Color_4fv[ 2 ][ 3 ] = 1.0f;

    ChangeScreen( SA_MAINMENU );

    m_enemies.reserve( 100 );
    m_enemyGarbage.reserve( 32 );
    m_bullets.reserve( 500 );
    m_enemyBullets.reserve( 1000 );
    m_bulletGarbage.reserve( 200 );
}

Road::~Road()
{
    ClearMapData();

    delete m_fontPauseTxt;
    delete m_fontGuiTxt;
    delete m_fontBig;
    delete m_speedFanRing;
    delete m_radar;

    glDeleteTextures( 1, &HUDtex );
    glDeleteTextures( 1, &ButtonTexture );
    glDeleteTextures( 1, &menu_background );
    glDeleteTextures( 1, &menu_background_overlay );
    glDeleteTextures( 3, cyber_ring_texture );
    glDeleteTextures( 1, &starfield_texture );
    for ( auto& it : m_mapsContainer ) {
        glDeleteTextures( 1, &it.preview_image );
    }
}

GLint Road::OnExecute()
{
    if ( !OnInit() ) {
        return -1;
    }

    m_thread = SDL_CreateThread( OnUpdateStatic, this );
    if ( !m_thread ) {
        std::cout << "-= Unable to start Update thread, terminating! =-\n"
                  << SDL_GetError() << "\n";
        OnCleanup();
        return 0;
    }

    SDL_Event Event{};
    while ( Running ) {
        while ( SDL_PollEvent( &Event ) ) {
            OnEvent( Event );
        }
        OnRender();
    }

    std::cout << "Waiting for update thread... ";
    SDL_WaitThread( m_thread, nullptr );
    std::cout << "done.\n";

    OnCleanup();
    return 0;
}

void Road::OnEvent( SDL_Event& Event )
{
    switch ( Event.type ) {
    case SDL_QUIT:
        Running = false;
        break;

    case SDL_KEYDOWN:
        OnKeyDown( Event.key.keysym.sym, Event.key.keysym.mod, Event.key.keysym.unicode );
        break;

    case SDL_KEYUP:
        OnKeyUp( Event.key.keysym.sym, Event.key.keysym.mod, Event.key.keysym.unicode );
        break;

    case SDL_VIDEORESIZE:
        SCREEN_WIDTH = Event.resize.w;
        SCREEN_HEIGHT = Event.resize.h;
        InitNewSurface( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, FULLSCREEN );
        OnResize( SCREEN_WIDTH, SCREEN_HEIGHT );
        break;

    case SDL_MOUSEBUTTONDOWN:
        if ( Event.button.button == SDL_BUTTON_LEFT ) {
            OnMouseClickLeft( Event.button.x, Event.button.y );
        }
        break;
    }
}

void Road::OnCleanup()
{
    //   Mix_HaltMusic();
    Mix_HaltChannel( -1 );

    std::cout << Mix_GetError() << "\n";
    Mix_FreeChunk( m_laser );
    Mix_FreeChunk( m_blaster );
    Mix_FreeChunk( m_torpedo );
    Mix_FreeChunk( m_click );
    Mix_CloseAudio();

    Mix_Quit();

    SDL_FreeSurface( m_display );
    SDL_Quit();
    SaveConfig();
}

bool Road::OnInit()
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO ) < 0 ) {
        std::cout << "Unable to init SDL\n"
                  << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32 );

    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0 );

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 2 );

    //   GLint audio_rate = 44100;
    //   Uint16 audio_format = AUDIO_S16SYS;
    //   GLint audio_channels = 2;
    //   GLint audio_buffers = 4096;
    //  Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)
    if ( Mix_OpenAudio( 22050, AUDIO_S16SYS, 2, 4096 ) != 0 ) {
        std::fprintf( stderr, "Unable to initialize audio: %s\n", Mix_GetError() );
        return false;
    }

    m_laser = Mix_LoadWAV( "sounds/laser.wav" );
    m_blaster = Mix_LoadWAV( "sounds/blaster.wav" );
    m_torpedo = Mix_LoadWAV( "sounds/torpedo.wav" );
    m_click = Mix_LoadWAV( "sounds/click.wav" );

    LoadConfig();

    if ( !InitNewSurface( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, FULLSCREEN ) ) {
        return false;
    }

    InitRoadAdditionsGL();
    OnResize( SCREEN_WIDTH, SCREEN_HEIGHT );
    return true;
}

void Road::OnResize( GLint w, GLint h )
{
    if ( w > h ) {
        max_dimention = w;
        min_dimention = h;
    }
    else {
        max_dimention = h;
        min_dimention = w;
    }

    glViewport( 0, 0, w, h );
    m_btnExit.UpdateCoord( ( SCREEN_WIDTH / 2 ) + 4, SCREEN_HEIGHT * 0.15 );
    m_btnQuitMission.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 196, SCREEN_HEIGHT * 0.15 );
    m_btnChangeFiltering.UpdateCoord( 512, SCREEN_HEIGHT - 192 );
    m_btnSelectMission.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 96, SCREEN_HEIGHT * 0.15 + 52 );
    m_btnGO.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 96, SCREEN_HEIGHT * 0.15 );
    m_btnStartMission.UpdateCoord( ( SCREEN_WIDTH / 2 ) + 4, SCREEN_HEIGHT * 0.15 );
    m_btnReturnToMainMenu.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 196, SCREEN_HEIGHT * 0.15 );
    m_btnReturnToMissionSelection.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 96, SCREEN_HEIGHT * 0.15 );
    m_btnNextMap.UpdateCoord( SCREEN_WIDTH - 240, SCREEN_HEIGHT / 2 - 24 );
    m_btnPrevMap.UpdateCoord( 48, SCREEN_HEIGHT / 2 - 24 );
    m_btnCustomize.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 196, SCREEN_HEIGHT * 0.15 );
    m_btnCustomizeReturn.UpdateCoord( ( SCREEN_WIDTH / 2 ) - 96, SCREEN_HEIGHT * 0.15 + 52 );
    m_btnNextJet.UpdateCoord( SCREEN_WIDTH - 240, SCREEN_HEIGHT / 2 - 24 );
    m_btnPrevJet.UpdateCoord( 48, SCREEN_HEIGHT / 2 - 24 );

    m_btnWeap1.UpdateCoord( SCREEN_WIDTH / 2 - 196 - 96, SCREEN_HEIGHT * 0.15 + 52 - 76 );
    m_btnWeap2.UpdateCoord( SCREEN_WIDTH / 2 - 96, SCREEN_HEIGHT * 0.15 + 52 - 76 );
    m_btnWeap3.UpdateCoord( SCREEN_WIDTH / 2 + 100, SCREEN_HEIGHT * 0.15 + 52 - 76 );
}

void Road::InitRoadAdditionsGL()
{
    //   init functons

    if ( TTF_Init() < 0 ) {
        std::cout << "Unable to initialize library: " << TTF_GetError() << "\n";
    }
    //     setTextureFiltering(FILTERING_ANISOTROPIC_X16);
    m_fontPauseTxt = new Font( "misc/DejaVuSans-Bold.ttf", 18 );
    m_fontGuiTxt = new Font( "misc/DejaVuSans-Bold.ttf", 12 );
    m_fontBig = new Font( "misc/DejaVuSans-Bold.ttf", 32 );
    TTF_Quit();

    ButtonTexture = LoadTexture( "textures/button1.tga" );

    m_btnChangeFiltering.SetFont( m_fontGuiTxt );
    m_btnChangeFiltering.SetText( "Anisotropic x16" );
    m_btnChangeFiltering.SetTexture( ButtonTexture );

    m_btnExit.SetFont( m_fontGuiTxt );
    m_btnExit.SetText( "Exit Game" );
    m_btnExit.SetTexture( ButtonTexture );

    m_btnSelectMission.SetFont( m_fontGuiTxt );
    m_btnSelectMission.SetText( "Select Mission" );
    m_btnSelectMission.SetTexture( ButtonTexture );

    m_btnQuitMission.SetFont( m_fontGuiTxt );
    m_btnQuitMission.SetText( "Quit Mission" );
    m_btnQuitMission.SetTexture( ButtonTexture );

    m_btnStartMission.SetFont( m_fontGuiTxt );
    m_btnStartMission.SetText( "Start Mission" );
    m_btnStartMission.SetTexture( ButtonTexture );

    m_btnReturnToMissionSelection.SetFont( m_fontGuiTxt );
    m_btnReturnToMissionSelection.SetText( "Return" );
    m_btnReturnToMissionSelection.SetTexture( ButtonTexture );

    m_btnReturnToMainMenu.SetFont( m_fontGuiTxt );
    m_btnReturnToMainMenu.SetText( "Return" );
    m_btnReturnToMainMenu.SetTexture( ButtonTexture );

    m_btnGO.SetFont( m_fontGuiTxt );
    m_btnGO.SetText( "GO!" );
    m_btnGO.SetTexture( ButtonTexture );

    m_btnNextMap.SetFont( m_fontGuiTxt );
    m_btnNextMap.SetText( "Next Map" );
    m_btnNextMap.SetTexture( ButtonTexture );

    m_btnPrevMap.SetFont( m_fontGuiTxt );
    m_btnPrevMap.SetText( "Previous Map" );
    m_btnPrevMap.SetTexture( ButtonTexture );

    m_btnNextJet.SetFont( m_fontGuiTxt );
    m_btnNextJet.SetText( "Next Jet" );
    m_btnNextJet.SetTexture( ButtonTexture );

    m_btnPrevJet.SetFont( m_fontGuiTxt );
    m_btnPrevJet.SetText( "Previous Jet" );
    m_btnPrevJet.SetTexture( ButtonTexture );

    m_btnCustomizeReturn.SetFont( m_fontGuiTxt );
    m_btnCustomizeReturn.SetText( "Done" );
    m_btnCustomizeReturn.SetTexture( ButtonTexture );

    m_btnCustomize.SetFont( m_fontGuiTxt );
    m_btnCustomize.SetText( "Customise" );
    m_btnCustomize.SetTexture( ButtonTexture );

    m_btnWeap1.SetFont( m_fontGuiTxt );
    m_btnWeap1.SetTexture( ButtonTexture );

    m_btnWeap2.SetFont( m_fontGuiTxt );
    m_btnWeap2.SetTexture( ButtonTexture );

    m_btnWeap3.SetFont( m_fontGuiTxt );
    m_btnWeap3.SetTexture( ButtonTexture );

    switch ( Weap1 ) {
    case 0:
        m_btnWeap1.SetText( "Laser" );
        break;
    case 1:
        m_btnWeap1.SetText( "Blaster" );
        break;
    case 2:
        m_btnWeap1.SetText( "Torpedo" );
        break;
    }

    switch ( Weap2 ) {
    case 0:
        m_btnWeap2.SetText( "Laser" );
        break;
    case 1:
        m_btnWeap2.SetText( "Blaster" );
        break;
    case 2:
        m_btnWeap2.SetText( "Torpedo" );
        break;
    }

    switch ( Weap3 ) {
    case 0:
        m_btnWeap3.SetText( "Laser" );
        break;
    case 1:
        m_btnWeap3.SetText( "Blaster" );
        break;
    case 2:
        m_btnWeap3.SetText( "Torpedo" );
        break;
    }

    TimePassed = time( nullptr );
    HUDtex = LoadTexture( "textures/HUDtex.tga" );

    menu_background = LoadTexture( "textures/background.tga" );
    menu_background_overlay = LoadTexture( "textures/background-overlay.tga" );
    starfield_texture = LoadTexture( "textures/star_field_transparent.tga" );
    alpha_value = 1;
    background_effect_equation = false;

    m_speedFanRing = new Circle( 32, 26 );

    cyber_ring_texture[ 0 ] = LoadTexture( "textures/cyber_ring1.tga" );
    cyber_ring_texture[ 1 ] = LoadTexture( "textures/cyber_ring2.tga" );
    cyber_ring_texture[ 2 ] = LoadTexture( "textures/cyber_ring3.tga" );
    //     cyber_ring_texture[3] = LoadTexture("textures/cyber_ring4.tga");
    cyber_ring_rotation[ 0 ] = 0;
    cyber_ring_rotation[ 1 ] = 0;
    cyber_ring_rotation[ 2 ] = 0;
    cyber_ring_rotation_direction[ 0 ] = false;
    cyber_ring_rotation_direction[ 1 ] = false;
    cyber_ring_rotation_direction[ 2 ] = false;

    GLfloat temp_colors[ 4 ][ 4 ] = { { 1, 1, 1, 0.8 }, { 1, 1, 1, 0.7 }, { 1, 1, 1, 0.6 }, { 1, 1, 1, 0.7 } };

    memcpy( cyber_ring_color, temp_colors, sizeof( GLfloat ) * 12 );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    LightAmbient[ 0 ] = 0.5f;
    LightAmbient[ 1 ] = 0.5f;
    LightAmbient[ 2 ] = 0.5f;
    LightAmbient[ 3 ] = 1.0f;

    LightDiffuse[ 0 ] = 0.8f;
    LightDiffuse[ 1 ] = 0.8f;
    LightDiffuse[ 2 ] = 0.8f;
    LightDiffuse[ 3 ] = 1.0f;

    LightPosition[ 0 ] = 0;
    LightPosition[ 1 ] = 1;
    LightPosition[ 2 ] = 1;
    LightPosition[ 3 ] = 1;

    glEnable( GL_CULL_FACE );
    glFrontFace( GL_CCW );
    glMaterialfv( GL_FRONT, GL_AMBIENT, LightAmbient );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, LightDiffuse );
    glLightfv( GL_LIGHT0, GL_AMBIENT, LightAmbient );

    glLightfv( GL_LIGHT0, GL_DIFFUSE, LightDiffuse );
    glLightfv( GL_LIGHT0, GL_POSITION, LightPosition );
    glEnable( GL_LIGHT0 );

    GLfloat fogColor[ 4 ] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glFogi( GL_FOG_MODE, GL_LINEAR );
    glFogfv( GL_FOG_COLOR, fogColor );
    glFogf( GL_FOG_DENSITY, 0.1f );
    glHint( GL_FOG_HINT, GL_NICEST );
    glFogf( GL_FOG_START, 10.0f );
    glFogf( GL_FOG_END, 25.0f );

    GLfloat tempcolor[ 4 ][ 4 ] = {
        { 0.3, 0.8, 1, 1 }, { 1, 0.8, 0.1, 1 }, { 1, 0.3, 0.8, 1 }, { 1, 1, 0, 1 }
    };
    BulletProto tmpWeapon;
    tmpWeapon.type = Bullet::SLUG;
    tmpWeapon.delay = 0.1;
    tmpWeapon.energy = 15;
    tmpWeapon.damage = 1;
    tmpWeapon.score_per_hit = 1;
    memcpy( tmpWeapon.color1, tempcolor[ 3 ], 4 * sizeof( GLfloat ) );
    memcpy( tmpWeapon.color2, tempcolor[ 1 ], 4 * sizeof( GLfloat ) );
    Weapons[ 0 ] = tmpWeapon;

    tmpWeapon.type = Bullet::BLASTER;
    tmpWeapon.speed = 16;
    tmpWeapon.damage = 10;
    tmpWeapon.energy = 10;
    tmpWeapon.delay = 0.2;
    memcpy( tmpWeapon.color1, tempcolor[ 0 ], 4 * sizeof( GLfloat ) );
    tmpWeapon.score_per_hit = 30;
    Weapons[ 1 ] = tmpWeapon;

    memcpy( tmpWeapon.color1, tempcolor[ 1 ], 4 * sizeof( GLfloat ) );
    tmpWeapon.delay = 0.4;
    Weapons[ 3 ] = tmpWeapon;

    tmpWeapon.type = Bullet::TORPEDO;
    tmpWeapon.damage = 1;
    tmpWeapon.delay = 0.4;
    tmpWeapon.energy = 1;
    tmpWeapon.speed = 8;
    tmpWeapon.score_per_hit = 2;
    memcpy( tmpWeapon.color1, tempcolor[ 2 ], 4 * sizeof( GLfloat ) );
    Weapons[ 2 ] = tmpWeapon;
}

void Road::UpdateCyberRings()
{
    cyber_ring_rotation[ 0 ] += 25.0 * DELTATIME;
    cyber_ring_rotation[ 1 ] -= 15.0 * DELTATIME;
    if ( cyber_ring_rotation_direction[ 2 ] ) {
        cyber_ring_rotation[ 2 ] += 35.0 * DELTATIME;
    }
    else {
        cyber_ring_rotation[ 2 ] -= 20.0 * DELTATIME;
    }
    //     cyber_ring_rotation[3] += 20.0 * DELTATIME;

    if ( cyber_ring_rotation[ 0 ] >= 360 ) {
        cyber_ring_rotation[ 0 ] -= 360;
    }
    if ( cyber_ring_rotation[ 1 ] < 0 ) {
        cyber_ring_rotation[ 1 ] += 360;
    }
    if ( cyber_ring_rotation[ 2 ] < 0 ) {
        cyber_ring_rotation_direction[ 2 ] = true;
    }
    if ( cyber_ring_rotation[ 2 ] > 90 ) {
        cyber_ring_rotation_direction[ 2 ] = false;
    }
    //     if (cyber_ring_rotation[3]>=360) { cyber_ring_rotation[3] -= 360; }
}

void Road::OnRender()
{
    timeS = SDL_GetTicks();
    switch ( SCREEN ) {
    case SA_GAMESCREEN:
        GameScreen();
        break;
    case SA_GAMESCREEN_PAUSED:
        GameScreenPaused();
        break;
    case SA_GAMESCREEN_BRIEFING:
        GameScreenBriefing();
        break;
    case SA_DEADSCREEN:
        DeadScreen();
        break;
    case SA_WINSCREEN:
        WinScreen();
        break;
    case SA_MISSIONSELECTION:
        MissionSelectionScreen();
        break;
    case SA_MAINMENU:
        DrawMainMenu();
        break;
    case SA_CUSTOMIZE:
        ScreenCustomize();
        break;
    default:
        break;
    }
    SDL_GL_SwapBuffers();
}

void Road::OnUpdate()
{
    while ( Running ) {
        switch ( SCREEN ) {
        case SA_GAMESCREEN:
            GameUpdate();
            break;
        case SA_GAMESCREEN_PAUSED:
            GameUpdatePaused();
            break;
        case SA_GAMESCREEN_BRIEFING:
            GameScreenBriefingUpdate();
            break;
        case SA_DEADSCREEN:
            DeadScreenUpdate();
            break;
        case SA_WINSCREEN:
            WinUpdate();
            break;
        case SA_MISSIONSELECTION:
            MissionSelectionUpdate();
            break;
        case SA_MAINMENU:
            UpdateMainMenu();
            break;
        case SA_CUSTOMIZE:
            UpdateCustomize();
            break;
        }
        SDL_Delay( Delay() );
    }
}

int Road::OnUpdateStatic( void* param )
{
    Road* r = reinterpret_cast<Road*>( param );
    r->OnUpdate();
    return 0;
}

Uint32 Road::Delay()
{
    static Uint32 next = 0;
    Uint32 now = SDL_GetTicks();
    if ( next <= now ) {
        next = now + Time_Interval;
        return 0;
    }
    return next - now;
}

void Road::Pause()
{
    ChangeScreen( SA_GAMESCREEN_PAUSED );
}

void Road::Unpause()
{
    ChangeScreen( SA_GAMESCREEN );
}

void Road::GameUpdatePaused()
{
    UpdateCyberRings();
}

void Road::GameUpdate()
{
    if ( m_jet->GetStatus() == SAObject::DEAD ) {
        ChangeScreen( SA_DEADSCREEN );
    }
    if ( m_enemies.empty() ) {
        ChangeScreen( SA_WINSCREEN );
    }
    if ( m_jet->GetHealth() <= 20 ) {
        HUD_Color = 2;
    }
    else {
        HUD_Color = 0;
    }

    if ( m_jet->IsShooting( 0 ) ) {
        AddBullet( 0 );
    }
    if ( m_jet->IsShooting( 1 ) ) {
        AddBullet( 1 );
    }
    if ( m_jet->IsShooting( 2 ) ) {
        AddBullet( 2 );
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( Enemy*& e : m_enemies ) {
            e->Update();
            if ( e->IsWeaponReady() ) {
                std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
                m_enemyBullets.push_back( e->GetWeapon() );
            }
            if ( e->GetStatus() == Enemy::DEAD ) {
                m_jet->AddScore( e->GetScore(), true );
                m_enemyGarbage.push_back( e );
                e = nullptr;
            }
        }
        m_enemies.erase( std::remove( m_enemies.begin(), m_enemies.end(), nullptr ), m_enemies.end() );
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
        for ( Bullet* it : m_enemyBullets ) {
            it->ProcessCollision( m_jet );
        }
    }

    m_jet->Update();
    speed_anim += m_jet->GetSpeed() * ( 270.0 * DELTATIME );
    if ( speed_anim >= 360 ) {
        speed_anim -= 360;
    }
    m_map->GetJetData( m_jet->GetPosition(), m_jet->GetVelocity() );
    m_map->Update();

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        std::lock_guard<std::mutex> lg2( m_mutexBullet );
        for ( Bullet*& b : m_bullets ) {
            for ( Enemy* e : m_enemies ) {
                b->ProcessCollision( e );
            }

            b->Update();
            if ( b->GetStatus() == Bullet::DEAD ) {
                m_bulletGarbage.push_back( b );
                b = nullptr;
                //         i-=1;
            }
        }
        m_bullets.erase( std::remove( m_bullets.begin(), m_bullets.end(), nullptr ), m_bullets.end() );
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
        for ( Bullet*& b : m_enemyBullets ) {
            b->Update();
            if ( b->GetStatus() == Bullet::DEAD ) {
                m_bulletGarbage.push_back( b );
                b = nullptr;
            }
        }
        m_enemyBullets.erase( std::remove( m_enemyBullets.begin(), m_enemyBullets.end(), nullptr ), m_enemyBullets.end() );
    }

    for ( Enemy*& e : m_enemyGarbage ) {
        if ( e->DeleteMe() ) {
            delete e;
            e = nullptr;
        }
    }
    m_enemyGarbage.erase( std::remove( m_enemyGarbage.begin(), m_enemyGarbage.end(), nullptr ), m_enemyGarbage.end() );

    for ( Bullet*& b : m_bulletGarbage ) {
        if ( b->DeleteMe() ) {
            delete b;
            b = nullptr;
        }
    }
    m_bulletGarbage.erase( std::remove( m_bulletGarbage.begin(), m_bulletGarbage.end(), nullptr ), m_bulletGarbage.end() );

    UpdateCyberRings();
}

void Road::AddBullet( GLuint wID )
{
    if ( !m_jet->IsWeaponReady( wID ) ) {
        return;
    }
    m_jet->TakeEnergy( wID );
    m_bullets.push_back( m_jet->GetWeaponType( wID ) );
    ShotsDone++;
    switch ( m_bullets.back()->GetType() ) {
    case Bullet::BLASTER:
        PlaySound( m_blaster );
        break;
    case Bullet::SLUG:
        PlaySound( m_laser );
        break;
    case Bullet::TORPEDO:
        PlaySound( m_torpedo );
        break;
    }
}

void Road::OnMouseClickLeft( GLint X, GLint Y )
{
    Y = SCREEN_HEIGHT - Y;
    switch ( SCREEN ) {
    case SA_GAMESCREEN_PAUSED:
        if ( m_btnQuitMission.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_DEADSCREEN );
            break;
        }

    case SA_MAINMENU:
        if ( m_btnSelectMission.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_MISSIONSELECTION );
            break;
        }
        if ( m_btnExit.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            Running = false;
            break;
        }
        if ( m_btnCustomize.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_CUSTOMIZE );
            break;
        }
        break;
    case SA_MISSIONSELECTION:
        if ( m_btnStartMission.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_GAMESCREEN_BRIEFING );
            break;
        }
        if ( m_btnReturnToMainMenu.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_MAINMENU );
            break;
        }
        if ( m_btnNextMap.IsClicked( X, Y ) ) {
            current_map++;
            if ( current_map == m_mapsContainer.size() - 1 ) {
                m_btnNextMap.Disable();
            }
            m_btnPrevMap.Enable();
            PlaySound( m_click );
            break;
        }
        if ( m_btnPrevMap.IsClicked( X, Y ) ) {
            current_map--;
            if ( current_map == 0 ) {
                m_btnPrevMap.Disable();
            }
            if ( m_mapsContainer.size() > 1 ) {
                m_btnNextMap.Enable();
            }
            PlaySound( m_click );
            break;
        }
        break;
    case SA_DEADSCREEN:
        if ( m_btnReturnToMissionSelection.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_MISSIONSELECTION );
            break;
        }
    case SA_WINSCREEN:
        if ( m_btnReturnToMissionSelection.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_MISSIONSELECTION );
            break;
        }
        break;
    case SA_GAMESCREEN_BRIEFING:
        if ( m_btnGO.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_GAMESCREEN );
            break;
        }
        break;
    case SA_CUSTOMIZE:
        if ( m_btnNextJet.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            current_jet++;
            if ( current_jet == m_jetsContainer.size() - 1 ) {
                m_btnNextJet.Disable();
            }
            m_btnPrevJet.Enable();
            preview_model.Load_OBJ( m_jetsContainer.at( current_jet ).model_file.c_str() );
            preview_model.CalculateNormal();
            preview_model.BindTexture( LoadTexture( m_jetsContainer.at( current_jet ).model_texture.c_str() ) );
            break;
        }
        if ( m_btnPrevJet.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            current_jet--;
            if ( current_jet == 0 ) {
                m_btnPrevJet.Disable();
            }
            if ( m_jetsContainer.size() > 1 ) {
                m_btnNextJet.Enable();
            }
            preview_model.Load_OBJ( m_jetsContainer.at( current_jet ).model_file.c_str() );
            preview_model.CalculateNormal();
            preview_model.BindTexture( LoadTexture( m_jetsContainer.at( current_jet ).model_texture.c_str() ) );
            break;
        }
        if ( m_btnCustomizeReturn.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            ChangeScreen( SA_MAINMENU );
            break;
        }
        if ( m_btnWeap1.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            Weap1++;
            if ( Weap1 == 3 ) {
                Weap1 = 0;
            }
            if ( Weap1 == 0 ) {
                m_btnWeap1.SetText( "Laser" );
            }
            if ( Weap1 == 1 ) {
                m_btnWeap1.SetText( "Blaster" );
            }
            if ( Weap1 == 2 ) {
                m_btnWeap1.SetText( "Torpedo" );
            }
            break;
        }
        if ( m_btnWeap2.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            Weap2++;
            if ( Weap2 == 3 ) {
                Weap2 = 0;
            }
            if ( Weap2 == 0 ) {
                m_btnWeap2.SetText( "Laser" );
            }
            if ( Weap2 == 1 ) {
                m_btnWeap2.SetText( "Blaster" );
            }
            if ( Weap2 == 2 ) {
                m_btnWeap2.SetText( "Torpedo" );
            }
            break;
        }
        if ( m_btnWeap3.IsClicked( X, Y ) ) {
            PlaySound( m_click );
            Weap3++;
            if ( Weap3 == 3 ) {
                Weap3 = 0;
            }
            if ( Weap3 == 0 ) {
                m_btnWeap3.SetText( "Laser" );
            }
            if ( Weap3 == 1 ) {
                m_btnWeap3.SetText( "Blaster" );
            }
            if ( Weap3 == 2 ) {
                m_btnWeap3.SetText( "Torpedo" );
            }
            break;
        }
        break;
    default:
        break;
    }
}

void Road::Retarget()
{
    std::lock_guard<std::mutex> lg( m_mutexEnemy );
    if ( m_enemies.empty() ) {
        return;
    }
    static std::mt19937_64 random{ std::random_device()() };
    m_jet->LockTarget( m_enemies[ random() % m_enemies.size() ] );
}

void Road::UpdateMainMenu()
{
    UpdateClouds();
    UpdateCyberRings();
}

void Road::GameScreenBriefingUpdate()
{
    GameUpdatePaused();
}

void Road::ClearMapData()
{
    std::cout << "Moving all enemies to garbage.\n";
    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( Enemy* e : m_enemies ) {
            delete e;
        }
        m_enemies.clear();
    }

    std::cout << "Moving all bullets to garbage.\n";
    {
        std::lock_guard<std::mutex> lg( m_mutexBullet );
        for ( Bullet* b : m_bullets ) {
            delete b;
        }
        m_bullets.clear();
    }

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemyBullet );
        for ( Bullet* b : m_enemyBullets ) {
            delete b;
        }
        m_enemyBullets.clear();
    }

    std::cout << "Cleaning garbage...\n";
    for ( Enemy* e : m_enemyGarbage ) {
        delete e;
    }
    m_enemyGarbage.clear();

    for ( Bullet* b : m_bulletGarbage ) {
        delete b;
    }
    m_bulletGarbage.clear();

    std::cout << "Cleaning garbage: done.\n";
    delete m_map;
    m_map = nullptr;
    delete m_jet;
    m_jet = nullptr;
}

void Road::CreateMapData( const MapProto& map_data, const ModelProto& model_data )
{
    ShotsDone = 0;
    HUD_Color = 0;
    m_jet = new Jet( model_data );
    m_map = new Map( map_data );
    m_jet->SetWeapon( Weapons[ Weap1 ], 0 );
    m_jet->SetWeapon( Weapons[ Weap2 ], 1 );
    m_jet->SetWeapon( Weapons[ Weap3 ], 2 );

    {
        std::lock_guard<std::mutex> lg( m_mutexEnemy );
        for ( GLuint i = 0; i < map_data.enemies; i++ ) {
            m_enemies.push_back( new Enemy() );
            m_enemies.back()->SetTarget( m_jet );
            m_enemies.back()->SetWeapon( Weapons[ 3 ] );
        }
    }

    for ( MapProto& it : m_mapsContainer ) {
        glDeleteTextures( 1, &it.preview_image );
        it.preview_image = 0;
    }
}

void Road::MissionSelectionUpdate()
{
    UpdateCyberRings();
}

void Road::ChangeScreen( GLubyte SCR )
{
    SDL_ShowCursor( SCR != SA_GAMESCREEN );

    switch ( SCR ) {
    case SA_GAMESCREEN:
    case SA_GAMESCREEN_PAUSED:
    case SA_DEADSCREEN:
    case SA_MAINMENU:
    case SA_WINSCREEN:
        SCREEN = SCR;
        break;

    case SA_GAMESCREEN_BRIEFING:
        CreateMapData( m_mapsContainer.at( current_map ), m_jetsContainer.at( current_jet ) );
        SCREEN = SCR;
        break;

    case SA_MISSIONSELECTION:
        ClearMapData();
        SCREEN = SCR;
        break;

    case SA_CUSTOMIZE:
        model_rotation = 135.0;
        preview_model.Load_OBJ( m_jetsContainer.at( current_jet ).model_file.c_str() );
        preview_model.BindTexture( LoadTexture( m_jetsContainer.at( current_jet ).model_texture.c_str() ) );
        preview_model.CalculateNormal();
        SCREEN = SCR;
        break;

    default:
        break;
    }
}

void Road::GoFullscreen( bool& b )
{
    b = !b;
    InitNewSurface( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, b );
}

bool Road::InitNewSurface( GLint W, GLint H, GLint D, bool F )
{
    SDL_Surface* tmp = m_display;
    SDL_Surface* tmp2 = nullptr;
    if ( F ) {
        tmp2 = SDL_SetVideoMode( W, H, D, SDL_DOUBLEBUF | SDL_OPENGL | SDL_FULLSCREEN );
    }
    else {
        tmp2 = SDL_SetVideoMode( W, H, D, SDL_DOUBLEBUF | SDL_OPENGL | SDL_RESIZABLE );
    }
    if ( !tmp2 ) {
        std::cout << "Unable to create display surface:\n";
        std::string error( SDL_GetError() );
        std::cout << error << "\n";
        return false;
    }
    m_display = tmp2;
    if ( tmp ) {
        SDL_FreeSurface( tmp );
    }
    return true;
}

void Road::LoadMapProto()
{
    std::cout << "Loadings maps... ";
    m_mapsContainer.clear();
    MapProto map;
    std::ifstream MapFile( "maps.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::string line;
    while ( getline( MapFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        if ( strcmp( value_1, "[MAP]" ) == 0 ) {
            m_mapsContainer.push_back( map );
        }
        if ( strcmp( value_1, "name" ) == 0 ) {
            m_mapsContainer.back().name = value_2;
        }
        if ( strcmp( value_1, "enemies" ) == 0 ) {
            m_mapsContainer.back().enemies = atoi( value_2 );
        }
        if ( strcmp( value_1, "top" ) == 0 ) {
            m_mapsContainer.back().TOP = value_2;
        }
        if ( strcmp( value_1, "bottom" ) == 0 ) {
            m_mapsContainer.back().BOTTOM = value_2;
        }
        if ( strcmp( value_1, "left" ) == 0 ) {
            m_mapsContainer.back().LEFT = value_2;
        }
        if ( strcmp( value_1, "right" ) == 0 ) {
            m_mapsContainer.back().RIGHT = value_2;
        }
        if ( strcmp( value_1, "front" ) == 0 ) {
            m_mapsContainer.back().FRONT = value_2;
        }
        if ( strcmp( value_1, "back" ) == 0 ) {
            m_mapsContainer.back().BACK = value_2;
        }
        if ( strcmp( value_1, "preview" ) == 0 ) {
            m_mapsContainer.back().preview_image_location = value_2;
        }
    }
    MapFile.close();
    if ( m_mapsContainer.empty() ) {
        m_mapsContainer.push_back( map );
        m_btnNextMap.Disable();
    }
    current_map = 0;
    m_btnPrevMap.Disable();
    std::cout << "done\n";
}

void Road::LoadJetProto()
{
    std::cout << "Loadings jets... ";
    m_jetsContainer.clear();
    ModelProto mod;
    std::ifstream JetFile( "jets.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::string line;
    while ( getline( JetFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        //     cout<<value_1<<" "<<value_2<<"\n";
        if ( strcmp( value_1, "[JET]" ) == 0 ) {
            m_jetsContainer.push_back( mod );
        }
        if ( strcmp( value_1, "name" ) == 0 ) {
            m_jetsContainer.back().name = value_2;
        }
        if ( strcmp( value_1, "texture" ) == 0 ) {
            m_jetsContainer.back().model_texture = value_2;
        }
        if ( strcmp( value_1, "model" ) == 0 ) {
            m_jetsContainer.back().model_file = value_2;
        }
        if ( strcmp( value_1, "scale" ) == 0 ) {
            m_jetsContainer.back().scale = atof( value_2 );
        }
    }
    JetFile.close();
    if ( m_jetsContainer.empty() ) {
        std::cout << "no jets\n";
        m_jetsContainer.push_back( mod );
    }
    if ( m_jetsContainer.size() == 1 ) {
        m_btnNextJet.Disable();
    }
    std::cout << "size " << m_jetsContainer.size() << "\n";
    current_jet = 0;
    for ( GLuint i = 0; i < m_jetsContainer.size(); i++ ) {
        if ( LastSelectedJetName == m_jetsContainer.at( i ).name ) {
            current_jet = i;
        }
    }
    if ( current_jet == 0 ) {
        m_btnPrevJet.Disable();
    }
    if ( current_jet == m_jetsContainer.size() - 1 ) {
        m_btnNextJet.Disable();
    }

    std::cout << "done\n";
}

void Road::LoadConfig()
{
    std::cout << "Loading from configuration file... ";
    std::ifstream ConfigFile( "config.cfg" );
    char value_1[ 48 ]{};
    char value_2[ 48 ]{};
    std::string line;
    while ( getline( ConfigFile, line ) ) {
        std::sscanf( line.c_str(), "%s %s", value_1, value_2 );
        if ( strcmp( value_1, "width" ) == 0 ) {
            SCREEN_WIDTH = atoi( value_2 );
        }
        if ( strcmp( value_1, "height" ) == 0 ) {
            SCREEN_HEIGHT = atoi( value_2 );
        }
        if ( strcmp( value_1, "fullscreen" ) == 0 ) {
            FULLSCREEN = atoi( value_2 ) != 0;
        }
        if ( strcmp( value_1, "texturefiltering" ) == 0 ) {
            current_filtering = atoi( value_2 );
        }
        if ( strcmp( value_1, "jet" ) == 0 ) {
            LastSelectedJetName = value_2;
        }
        if ( strcmp( value_1, "weap1" ) == 0 ) {
            if ( strcmp( value_2, "laser" ) == 0 ) {
                Weap1 = 0;
            }
            if ( strcmp( value_2, "blaster" ) == 0 ) {
                Weap1 = 1;
            }
            if ( strcmp( value_2, "torpedo" ) == 0 ) {
                Weap1 = 2;
            }
        }
        if ( strcmp( value_1, "weap2" ) == 0 ) {
            if ( strcmp( value_2, "laser" ) == 0 ) {
                Weap2 = 0;
            }
            if ( strcmp( value_2, "blaster" ) == 0 ) {
                Weap2 = 1;
            }
            if ( strcmp( value_2, "torpedo" ) == 0 ) {
                Weap2 = 2;
            }
        }
        if ( strcmp( value_1, "weap3" ) == 0 ) {
            if ( strcmp( value_2, "laser" ) == 0 ) {
                Weap3 = 0;
            }
            if ( strcmp( value_2, "blaster" ) == 0 ) {
                Weap3 = 1;
            }
            if ( strcmp( value_2, "torpedo" ) == 0 ) {
                Weap3 = 2;
            }
        }
        if ( strcmp( value_1, "sound" ) == 0 ) {
            play_sound = atoi( value_2 ) != 0;
        }
    }
    ConfigFile.close();
    std::cout << "done.\n";
    LoadMapProto();
    LoadJetProto();
}

void Road::SaveConfig()
{
    std::cout << "Saving to configuration file... ";
    std::ofstream ConfigFile( "config.cfg" );
    ConfigFile << "width " << SCREEN_WIDTH << "\n";
    ConfigFile << "height " << SCREEN_HEIGHT << "\n";
    ConfigFile << "fullscreen " << FULLSCREEN << "\n";
    ConfigFile << "texturefiltering " << current_filtering << "\n";
    ConfigFile << "sound " << play_sound << "\n";
    ConfigFile << "jet " << m_jetsContainer.at( current_jet ).name << "\n";
    ConfigFile << "weap1 ";
    switch ( Weap1 ) {
    case 0:
        ConfigFile << "laser\n";
        break;
    case 1:
        ConfigFile << "blaster\n";
        break;
    case 2:
        ConfigFile << "torpedo\n";
        break;
    }
    ConfigFile << "weap2 ";
    switch ( Weap2 ) {
    case 0:
        ConfigFile << "laser\n";
        break;
    case 1:
        ConfigFile << "blaster\n";
        break;
    case 2:
        ConfigFile << "torpedo\n";
        break;
    }
    ConfigFile << "weap3 ";
    switch ( Weap3 ) {
    case 0:
        ConfigFile << "laser\n";
        break;
    case 1:
        ConfigFile << "blaster\n";
        break;
    case 2:
        ConfigFile << "torpedo\n";
        break;
    }
    ConfigFile.close();
    std::cout << "done.\n";
}

void Road::UpdateClouds()
{
    if ( background_effect_equation ) {
        alpha_value += 0.1 * DELTATIME;
    }
    else {
        alpha_value -= 0.1 * DELTATIME;
    }

    if ( alpha_value >= 1 ) {
        background_effect_equation = false;
    }
    if ( alpha_value <= 0.2 ) {
        background_effect_equation = true;
    }
}

void Road::SetOrtho() const
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -192, 192 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void Road::SetPerspective( GLdouble Angle ) const
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( Angle, static_cast<GLdouble>( SCREEN_WIDTH ) / static_cast<GLdouble>( SCREEN_HEIGHT ), 0.001, 2000 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
}

void Road::UpdateCustomize()
{
    UpdateClouds();
    UpdateCyberRings();
    model_rotation += 30.0 * DELTATIME;
    if ( model_rotation >= 360.0 ) {
        model_rotation -= 360.0;
    }
}

void Road::PlaySound( Mix_Chunk* sound ) const
{
    if ( play_sound ) {
        Mix_Playing( Mix_PlayChannel( -1, sound, 0 ) );
    }
}

void Road::WinUpdate()
{
    //   UpdateClouds();
    UpdateCyberRings();
}

void Road::DeadScreenUpdate()
{
    UpdateCyberRings();
}
