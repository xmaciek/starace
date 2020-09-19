#include "jet.hpp"

#include <cassert>

Jet::Jet( const ModelProto& model_data )
{
    status = ALIVE;
    health = 100;
    position.x = 0;
    position.y = 0;
    position.z = 0;
    direction.x = 0;
    direction.y = 0;
    direction.z = -1;
    shotfactor[ 0 ] = 0;
    shotfactor[ 1 ] = 0;
    shotfactor[ 2 ] = 0;
    energy = 100;
    speed = 2;
    speed_acc = 0;
    norm_speed = 2;
    min_speed = 0.5;
    max_speed = 5;

    accX = 0;
    accY = 0;
    accZ = 0;
    rotX = 0;
    rotY = 0;
    rotZ = 0;
    pitch = 0;
    yaw = 0;
    roll = 0;

    btn_roll_left = btn_roll_right = btn_yaw_left = btn_yaw_right = btn_pitch_up = btn_pitch_down = false;

    shooting[ 0 ] = false;
    shooting[ 1 ] = false;
    shooting[ 2 ] = false;
    maxangleZ = 15;
    maxangleY = 5;
    maxangleX = 5;
    anglespeedZ = 50;
    anglespeedY = 10;
    anglespeedX = 15;
    model.Load_OBJ( model_data.model_file.c_str() );
    model.BindTexture( LoadTexture( model_data.model_texture.c_str() ) );
    model.Scale( model_data.scale );
    model.CalculateNormal();
    //       temprotshield = 0;
    target_locked[ 0 ] = false;
    target_locked[ 1 ] = false;
    target_locked[ 2 ] = false;
    shield = new Shield( 0.15, 0.03 );
    thruster = new Thruster( model_data.scale, model_data.scale * 0.04285 );
    GLfloat tmpcolor[ 4 ][ 4 ] = {
        { 0.0f, 0.7f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.5f, 0.0f },
        { 0.0f, 0.75f, 1.0f, 0.6f },
        { 0.0f, 0.0f, 0.5f, 0.0f }
    };

    CollisionDistance = 0.08;
    CollisionFlag = true;

    score = 0;

    thruster->SetColor( 1, tmpcolor[ 0 ] );
    thruster->SetColor( 0, tmpcolor[ 1 ] );
    thruster->SetColor( 3, tmpcolor[ 2 ] );
    thruster->SetColor( 2, tmpcolor[ 3 ] );

    crosshair = new Circle( 32, 0.06 );
};

Jet::~Jet()
{
    delete thruster;
    delete shield;
    delete crosshair;
}

void Jet::Draw()
{
    glPushMatrix();

    //       glColor3f(1,1,1);
    GLfloat matrix[ 16 ]{};
    //       Vertex v = quaternion.GetVector();
    //       glBegin(GL_LINES);
    //         glVertex3d(0,0,0);
    //         glVertex3d(direction.x, direction.y, direction.z);
    //       glEnd();
    animation.CreateMatrix( matrix );
    glMultMatrixf( matrix );

    model.Draw();
    for ( const auto& it : model.thrusters ) {
        thruster->DrawAt( it.x, it.y, it.z );
    }
    glPopMatrix();
}

void Jet::LockTarget( SAObject* t )
{
    if ( target ) {
        target->TargetMe( false );
    }

    target = t;
    target->TargetMe( true );
}

void Jet::Update()
{
    if ( btn_roll_left ) {
        if ( rotZ < maxangleZ ) {
            rotZ += anglespeedZ * DELTATIME;
        }
    }
    if ( btn_roll_right ) {
        if ( rotZ > -maxangleZ ) {
            rotZ -= anglespeedZ * DELTATIME;
        }
    }
    if ( ( !btn_roll_left && !btn_roll_right ) || ( btn_roll_left && btn_roll_right ) ) {
        roll = 0;
        if ( rotZ < -0.1 * DELTATIME ) {
            rotZ += anglespeedZ * DELTATIME;
        }
        if ( rotZ > 0.1 * DELTATIME ) {
            rotZ -= anglespeedZ * DELTATIME;
        }
    }
    else {
        if ( btn_roll_right ) {
            roll = 60;
        }
        else {
            roll = -60;
        }
    }

    if ( btn_yaw_left ) {
        if ( rotY < maxangleY ) {
            rotY += anglespeedY * DELTATIME;
        }
    }
    if ( btn_yaw_right ) {
        if ( rotY > -maxangleY ) {
            rotY -= anglespeedY * DELTATIME;
        }
    }
    if ( ( !btn_yaw_left && !btn_yaw_right ) || ( btn_yaw_left && btn_yaw_right ) ) {
        yaw = 0;
        if ( rotY < -0.1 * DELTATIME ) {
            rotY += anglespeedY * DELTATIME;
        }
        if ( rotY > 0.1 * DELTATIME ) {
            rotY -= anglespeedY * DELTATIME;
        }
    }
    else {
        if ( btn_yaw_right ) {
            yaw = 20;
        }
        else {
            yaw = -20;
        }
    }

    if ( btn_pitch_up ) {
        if ( rotX < maxangleX ) {
            rotX += anglespeedX * DELTATIME;
        }
    }
    if ( btn_pitch_down ) {
        if ( rotX > -maxangleX ) {
            rotX -= anglespeedX * DELTATIME;
        }
    }
    if ( ( !btn_pitch_up && !btn_pitch_down ) || ( btn_pitch_up && btn_pitch_down ) ) {
        pitch = 0;
        if ( rotX < -0.1 * DELTATIME ) {
            rotX += anglespeedX * DELTATIME;
        }
        if ( rotX > 0.1 * DELTATIME ) {
            rotX -= anglespeedX * DELTATIME;
        }
    }
    else {
        if ( btn_pitch_up ) {
            pitch = 40;
        }
        else {
            pitch = -40;
        }
    }

    //     if ((rotZ<maxangleZ) && (accZ>0)) { rotZ += anglespeedZ*DELTATIME; }
    //       else { if ((rotZ>-maxangleZ) && (accZ<0)) { rotZ -= anglespeedZ*DELTATIME; } }
    //     if (((rotZ>=0.1*DELTATIME) || (rotZ<=-0.1*DELTATIME)) && (accZ==0)) {
    //       if (rotZ<0) { rotZ += anglespeedZ*DELTATIME; } else { rotZ -= anglespeedZ*DELTATIME; }
    //     }
    //
    //     if ((rotX<maxangleX) && (accX>0)) { rotX += anglespeedX*DELTATIME; }
    //       else { if ((rotX>-maxangleX) && (accX<0)) { rotX -= anglespeedX*DELTATIME; } }
    //     if (((rotX>=0.1*DELTATIME) || (rotX<=-0.1*DELTATIME)) && (accX==0)) {
    //       if (rotX<0) { rotX += anglespeedX*DELTATIME; } else { rotX -= anglespeedX*DELTATIME; }
    //     }
    //
    //     if ((rotY<maxangleY) && (accY>0)) { rotY += anglespeedY*DELTATIME; }
    //       else { if ((rotY>-maxangleY) && (accY<0)) { rotY -= anglespeedY*DELTATIME; } }
    //     if (((rotY>=0.1*DELTATIME) || (rotY<=-0.1*DELTATIME)) && (accY==0)) {
    //       if (rotY<0) { rotY += anglespeedY*DELTATIME; } else { rotY -= anglespeedY*DELTATIME; }
    //     }

    if ( ( speed < max_speed ) && ( speed_acc > 0 ) ) {
        speed += 0.8 * DELTATIME;
    }
    else {
        if ( ( speed > min_speed ) && ( speed_acc < 0 ) ) {
            speed -= 0.8 * DELTATIME;
        }
    }
    if ( ( ( speed >= norm_speed + 0.1 * DELTATIME ) || ( speed <= norm_speed - 0.1 * DELTATIME ) ) && ( speed_acc == 0 ) ) {
        if ( speed < norm_speed ) {
            speed += 0.3 * DELTATIME;
        }
        else {
            speed -= 0.3 * DELTATIME;
        }
    }
    if ( speed < 3 ) {
        thruster->SetLength( speed / 8 );
    }
    Quaternion qtmp{};
    Quaternion qx{};
    Quaternion qy{};
    Quaternion qz{};

    qx.CreateFromAngles( 1, 0, 0, rotX );
    qy.CreateFromAngles( 0, 1, 0, -rotY );
    qz.CreateFromAngles( 0, 0, 1, -rotZ );
    qtmp = qy * qtmp; /* yaw */
    qtmp = qx * qtmp; /* pitch */
    qtmp = qz * qtmp; /* roll */
    animation = qtmp;

    //     qx.CreateFromAngles(1, 0, 0, pitch*DELTATIME);
    //     qy.CreateFromAngles(0, 1, 0, yaw*DELTATIME);
    //     qz.CreateFromAngles(0, 0, 1, roll*DELTATIME);
    qx.CreateFromAngles( 1, 0, 0, rotX * 4 * DELTATIME );
    qy.CreateFromAngles( 0, 1, 0, -rotY * 2.5 * DELTATIME );
    qz.CreateFromAngles( 0, 0, 1, -rotZ * 2.5 * DELTATIME );
    qtmp = quaternion;
    qtmp = qy * qtmp; /* yaw */
    qtmp = qx * qtmp; /* pitch */
    qtmp = qz * qtmp; /* roll */
    quaternion = qtmp;
    qtmp.Conjugate();
    rotation = qtmp;

    Vertex v;
    v.x = 0;
    v.y = 0;
    v.z = -1;
    quaternion.RotateVector( v );
    direction = v;
    normalise_v( direction );
    velocity = direction * speed;

    //     if (accX!=0) {
    //       if (accX>0) { pitch = -45.0; } else { pitch = 45.0; }
    //     } else { pitch = 0; }
    //
    //
    //     if (accY!=0) {
    //       if (accY>0) { yaw = 20.0; } else { yaw = -20.0; }
    //     } else { yaw = 0; }
    //
    //
    //     if (accZ!=0) {
    //       if (accZ>0) { roll = 60.0; } else { roll = -60.0; }
    //     } else { roll = 0; }

    if ( shotfactor[ 0 ] < Weapon[ 0 ].delay ) {
        shotfactor[ 0 ] += 1.0 * DELTATIME;
    }
    if ( shotfactor[ 1 ] < Weapon[ 1 ].delay ) {
        shotfactor[ 1 ] += 1.0 * DELTATIME;
    }
    if ( shotfactor[ 2 ] < Weapon[ 2 ].delay ) {
        shotfactor[ 2 ] += 1.0 * DELTATIME;
    }
    if ( energy < 100 ) {
        energy += 60 * DELTATIME;
    }
    else if ( energy > 100 ) {
        energy = 100;
    }

    position = position + velocity * DELTATIME;
    thruster->Update();
    shield->Update();
    if ( target ) {
        if ( target->GetStatus() != ALIVE ) {
            target->TargetMe( false );
            target = nullptr;
        }
    }
}

void Jet::RollLeft( bool doit )
{
    btn_roll_left = doit;
}

void Jet::RollRight( bool doit )
{
    btn_roll_right = doit;
}

void Jet::YawLeft( bool doit )
{
    btn_yaw_left = doit;
}

void Jet::YawRight( bool doit )
{
    btn_yaw_right = doit;
}

void Jet::PitchUp( bool doit )
{
    btn_pitch_up = doit;
}

void Jet::PitchDown( bool doit )
{
    btn_pitch_down = doit;
}

void Jet::SpeedUp( bool doit )
{
    speed_acc += doit ? 1 : -1;
}

void Jet::SpeedDown( bool doit )
{
    SpeedUp( !doit );
}

bool Jet::IsShooting( GLuint WeaponNum )
{
    return shooting[ WeaponNum ];
}

void Jet::Shoot( GLuint WeaponNum, bool doit )
{
    shooting[ WeaponNum ] = doit;
}

Vertex Jet::GetWeaponPoint( GLuint wID )
{
    Vertex w = model.weapons[ wID ];
    quaternion.RotateVector( w );
    w = w + position;
    return w;
}

Bullet* Jet::GetWeaponType( GLuint wID )
{
    BulletProto tmp = Weapon[ wID ];
    Vertex w = model.weapons[ wID ];
    quaternion.RotateVector( w );
    w = w + position;

    tmp.x = w.x;
    tmp.y = w.y;
    tmp.z = w.z;

    Bullet* b = new Bullet( tmp );
    b->SetDirection( direction );
    if ( tmp.type == Bullet::TORPEDO ) {
        if ( target ) {
            if ( target->GetStatus() == ALIVE ) {
                b->SetTarget( target );
            }
        }
    }
    return b;
}

void Jet::SetWeapon( BulletProto bp, GLuint ID )
{
    Weapon[ ID ] = bp;
}

bool Jet::IsWeaponReady( GLuint WeaponNum )
{
    return ( shotfactor[ WeaponNum ] >= Weapon[ WeaponNum ].delay ) && ( energy >= Weapon[ WeaponNum ].energy );
}

void Jet::TakeEnergy( GLuint wID )
{
    energy -= Weapon[ wID ].energy;
    shotfactor[ wID ] = 0;
}

void Jet::DrawWireframe()
{
    model.DrawWireframe();
}

void Jet::ProcessCollision( std::vector<Bullet*>& Bullets )
{
    if ( status == DEAD ) {
        return;
    }
    for ( Bullet* it : Bullets ) {
        if ( it->GetStatus() != ALIVE ) { continue; }
        if ( distance_v( position, it->GetPosition() ) > 0.1 ) { continue; }
        health -= it->getDamage();
        it->Kill();
        if ( health <= 0 ) {
            status = DEAD;
            return;
        }
    }
}

void Jet::ProcessCollision( SAObject* )
{
    assert( !"shall not be called" );
};

void Jet::AddScore( GLint s, bool b )
{
    if ( b ) {
        score += s;
    }
}
