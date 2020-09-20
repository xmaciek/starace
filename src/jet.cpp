#include "jet.hpp"

#include <algorithm>
#include <cassert>

Jet::Jet( const ModelProto& model_data )
{
    CollisionDistance = 0.08;
    CollisionFlag = true;
    direction.x = 0;
    direction.y = 0;
    direction.z = -1;
    health = 100;
    position.x = 0;
    position.y = 0;
    position.z = 0;
    score = 0;
    speed = 2;
    status = ALIVE;

    m_model.loadOBJ( model_data.model_file.c_str() );
    m_model.bindTexture( LoadTexture( model_data.model_texture.c_str() ) );
    m_model.scale( model_data.scale );
    m_model.calculateNormal();

    m_shield = new Shield( 0.15, 0.03 );
    m_thruster = new Thruster( model_data.scale, model_data.scale * 0.04285 );
    GLfloat tmpcolor[ 4 ][ 4 ] = {
        { 0.0f, 0.7f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.5f, 0.0f },
        { 0.0f, 0.75f, 1.0f, 0.6f },
        { 0.0f, 0.0f, 0.5f, 0.0f }
    };

    m_thruster->setColor( 1, tmpcolor[ 0 ] );
    m_thruster->setColor( 0, tmpcolor[ 1 ] );
    m_thruster->setColor( 3, tmpcolor[ 2 ] );
    m_thruster->setColor( 2, tmpcolor[ 3 ] );

    m_crosshair = new Circle( 32, 0.06 );
};

Jet::~Jet()
{
    delete m_thruster;
    delete m_shield;
    delete m_crosshair;
}

void Jet::Draw() const
{
    GLfloat matrix[ 16 ]{};
    m_animation.createMatrix( matrix );

    glPushMatrix();
    glMultMatrixf( matrix );

    m_model.draw();
    for ( const auto& it : m_model.thrusters() ) {
        m_thruster->drawAt( it.x, it.y, it.z );
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
    if ( m_btnRollLeft ) {
        if ( m_rotZ < m_maxAngleZ ) {
            m_rotZ += m_angleSpeedZ * DELTATIME;
        }
    }
    if ( m_btnRollRight ) {
        if ( m_rotZ > -m_maxAngleZ ) {
            m_rotZ -= m_angleSpeedZ * DELTATIME;
        }
    }
    if ( ( !m_btnRollLeft && !m_btnRollRight ) || ( m_btnRollLeft && m_btnRollRight ) ) {
        m_roll = 0;
        if ( m_rotZ < -0.1 * DELTATIME ) {
            m_rotZ += m_angleSpeedZ * DELTATIME;
        }
        if ( m_rotZ > 0.1 * DELTATIME ) {
            m_rotZ -= m_angleSpeedZ * DELTATIME;
        }
    }
    else {
        if ( m_btnRollRight ) {
            m_roll = 60;
        }
        else {
            m_roll = -60;
        }
    }

    if ( m_btnYawLeft ) {
        if ( m_rotY < m_maxAngleY ) {
            m_rotY += m_angleSpeedY * DELTATIME;
        }
    }
    if ( m_btnYawRight ) {
        if ( m_rotY > -m_maxAngleY ) {
            m_rotY -= m_angleSpeedY * DELTATIME;
        }
    }
    if ( ( !m_btnYawLeft && !m_btnYawRight ) || ( m_btnYawLeft && m_btnYawRight ) ) {
        m_yaw = 0;
        if ( m_rotY < -0.1 * DELTATIME ) {
            m_rotY += m_angleSpeedY * DELTATIME;
        }
        if ( m_rotY > 0.1 * DELTATIME ) {
            m_rotY -= m_angleSpeedY * DELTATIME;
        }
    }
    else {
        if ( m_btnYawRight ) {
            m_yaw = 20;
        }
        else {
            m_yaw = -20;
        }
    }

    if ( m_btnPitchUp ) {
        if ( m_rotX < m_maxAngleX ) {
            m_rotX += m_angleSpeedX * DELTATIME;
        }
    }
    if ( m_btnPitchDown ) {
        if ( m_rotX > -m_maxAngleX ) {
            m_rotX -= m_angleSpeedX * DELTATIME;
        }
    }
    if ( ( !m_btnPitchUp && !m_btnPitchDown ) || ( m_btnPitchUp && m_btnPitchDown ) ) {
        m_pitch = 0;
        if ( m_rotX < -0.1 * DELTATIME ) {
            m_rotX += m_angleSpeedX * DELTATIME;
        }
        if ( m_rotX > 0.1 * DELTATIME ) {
            m_rotX -= m_angleSpeedX * DELTATIME;
        }
    }
    else {
        if ( m_btnPitchUp ) {
            m_pitch = 40;
        }
        else {
            m_pitch = -40;
        }
    }

    if ( ( speed < m_maxSpeed ) && ( m_speedAcc > 0 ) ) {
        speed += 0.8 * DELTATIME;
    }
    else {
        if ( ( speed > m_minSpeed ) && ( m_speedAcc < 0 ) ) {
            speed -= 0.8 * DELTATIME;
        }
    }
    if ( ( ( speed >= m_normSpeed + 0.1 * DELTATIME ) || ( speed <= m_normSpeed - 0.1 * DELTATIME ) ) && ( m_speedAcc == 0 ) ) {
        if ( speed < m_normSpeed ) {
            speed += 0.3 * DELTATIME;
        }
        else {
            speed -= 0.3 * DELTATIME;
        }
    }
    if ( speed < 3 ) {
        m_thruster->setLength( speed / 8 );
    }
    Quaternion qtmp{};
    Quaternion qx{};
    Quaternion qy{};
    Quaternion qz{};

    qx.createFromAngles( 1, 0, 0, m_rotX );
    qy.createFromAngles( 0, 1, 0, -m_rotY );
    qz.createFromAngles( 0, 0, 1, -m_rotZ );
    qtmp = qy * qtmp; /* yaw */
    qtmp = qx * qtmp; /* pitch */
    qtmp = qz * qtmp; /* roll */
    m_animation = qtmp;

    qx.createFromAngles( 1, 0, 0, m_rotX * 4 * DELTATIME );
    qy.createFromAngles( 0, 1, 0, -m_rotY * 2.5 * DELTATIME );
    qz.createFromAngles( 0, 0, 1, -m_rotZ * 2.5 * DELTATIME );
    qtmp = m_quaternion;
    qtmp = qy * qtmp; /* yaw */
    qtmp = qx * qtmp; /* pitch */
    qtmp = qz * qtmp; /* roll */
    m_quaternion = qtmp;
    qtmp.conjugate();
    m_rotation = qtmp;

    Vertex v;
    v.x = 0;
    v.y = 0;
    v.z = -1;
    m_quaternion.rotateVector( v );
    direction = v;
    normalise_v( direction );
    velocity = direction * speed;

    if ( m_shotFactor[ 0 ] < m_weapon[ 0 ].delay ) {
        m_shotFactor[ 0 ] += 1.0 * DELTATIME;
    }
    if ( m_shotFactor[ 1 ] < m_weapon[ 1 ].delay ) {
        m_shotFactor[ 1 ] += 1.0 * DELTATIME;
    }
    if ( m_shotFactor[ 2 ] < m_weapon[ 2 ].delay ) {
        m_shotFactor[ 2 ] += 1.0 * DELTATIME;
    }

    m_energy = std::min( m_energy + 60 * DELTATIME, 100.0 );

    position = position + velocity * DELTATIME;
    m_thruster->update();
    m_shield->update();
    if ( target ) {
        if ( target->GetStatus() != ALIVE ) {
            target->TargetMe( false );
            target = nullptr;
        }
    }
}

void Jet::RollLeft( bool doit )
{
    m_btnRollLeft = doit;
}

void Jet::RollRight( bool doit )
{
    m_btnRollRight = doit;
}

void Jet::YawLeft( bool doit )
{
    m_btnYawLeft = doit;
}

void Jet::YawRight( bool doit )
{
    m_btnYawRight = doit;
}

void Jet::PitchUp( bool doit )
{
    m_btnPitchUp = doit;
}

void Jet::PitchDown( bool doit )
{
    m_btnPitchDown = doit;
}

void Jet::SpeedUp( bool doit )
{
    m_speedAcc += doit ? 1 : -1;
}

void Jet::SpeedDown( bool doit )
{
    SpeedUp( !doit );
}

bool Jet::IsShooting( GLuint WeaponNum )
{
    return m_shooting[ WeaponNum ];
}

void Jet::Shoot( GLuint WeaponNum, bool doit )
{
    m_shooting[ WeaponNum ] = doit;
}

Vertex Jet::GetWeaponPoint( GLuint wID )
{
    Vertex w = m_model.weapon( wID );
    m_quaternion.rotateVector( w );
    w = w + position;
    return w;
}

Bullet* Jet::GetWeaponType( GLuint wID )
{
    BulletProto tmp = m_weapon[ wID ];
    Vertex w = m_model.weapon( wID );
    m_quaternion.rotateVector( w );
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
    m_weapon[ ID ] = bp;
}

bool Jet::IsWeaponReady( GLuint WeaponNum )
{
    return ( m_shotFactor[ WeaponNum ] >= m_weapon[ WeaponNum ].delay )
        && ( m_energy >= m_weapon[ WeaponNum ].energy );
}

void Jet::TakeEnergy( GLuint wID )
{
    m_energy -= m_weapon[ wID ].energy;
    m_shotFactor[ wID ] = 0;
}

void Jet::DrawWireframe()
{
    m_model.drawWireframe();
}

void Jet::ProcessCollision( std::vector<Bullet*>& Bullets )
{
    if ( status == DEAD ) {
        return;
    }
    for ( Bullet* it : Bullets ) {
        if ( it->GetStatus() != ALIVE ) {
            continue;
        }
        if ( distance_v( position, it->GetPosition() ) > 0.1 ) {
            continue;
        }
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

double Jet::energy() const
{
    return m_energy;
}

Quaternion Jet::animation() const
{
    return m_animation;
}

Quaternion Jet::quat() const
{
    return m_quaternion;
}

Quaternion Jet::rotation() const
{
    return m_rotation;
}
