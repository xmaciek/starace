#include "jet.hpp"

#include "texture.hpp"

#include <algorithm>
#include <cassert>


Jet::Jet( const ModelProto& modelData )
: m_crosshair( 32, 0.06 )
, m_thruster( modelData.scale, modelData.scale * 0.04285 )
, m_shield( 0.15, 0.03 )
{
    m_collisionDistance = 0.08;
    m_collisionFlag = true;
    m_direction.z = -1;
    m_health = 100;
    m_speed = 2;
    setStatus( Status::eAlive );

    m_model.loadOBJ( modelData.model_file.c_str() );
    m_model.bindTexture( loadTexture( modelData.model_texture.c_str() ) );
    m_model.scale( modelData.scale );
    m_model.calculateNormal();

    float tmpcolor[ 4 ][ 4 ] = {
        { 0.0f, 0.7f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.5f, 0.0f },
        { 0.0f, 0.75f, 1.0f, 0.6f },
        { 0.0f, 0.0f, 0.5f, 0.0f }
    };

    m_thruster.setColor( 1, tmpcolor[ 0 ] );
    m_thruster.setColor( 0, tmpcolor[ 1 ] );
    m_thruster.setColor( 3, tmpcolor[ 2 ] );
    m_thruster.setColor( 2, tmpcolor[ 3 ] );

};


void Jet::draw() const
{
    float matrix[ 16 ]{};
    m_animation.createMatrix( matrix );

    glPushMatrix();
    glMultMatrixf( matrix );

    m_model.draw();
    for ( const auto& it : m_model.thrusters() ) {
        m_thruster.drawAt( it.x, it.y, it.z );
    }
    glPopMatrix();
}

void Jet::lockTarget( SAObject* t )
{
    if ( m_target ) {
        m_target->targetMe( false );
    }

    m_target = t;
    m_target->targetMe( true );
}

void Jet::update()
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

    if ( ( speed() < m_maxSpeed ) && ( m_speedAcc > 0 ) ) {
        m_speed += 0.8 * DELTATIME;
    }
    else {
        if ( ( m_speed > m_minSpeed ) && ( m_speedAcc < 0 ) ) {
            m_speed -= 0.8 * DELTATIME;
        }
    }
    if ( ( ( m_speed >= m_normSpeed + 0.1 * DELTATIME ) || ( speed() <= m_normSpeed - 0.1 * DELTATIME ) ) && ( m_speedAcc == 0 ) ) {
        if ( speed() < m_normSpeed ) {
            m_speed += 0.3 * DELTATIME;
        }
        else {
            m_speed -= 0.3 * DELTATIME;
        }
    }
    if ( speed() < 3 ) {
        m_thruster.setLength( speed() / 8 );
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
    m_direction = v;
    normalizeV( m_direction );
    m_velocity = direction() * speed();

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

    m_position += velocity() * DELTATIME;
    m_thruster.update();
    m_shield.update();
    if ( m_target ) {
        if ( m_target->status() != Status::eAlive ) {
            m_target->targetMe( false );
            m_target = nullptr;
        }
    }
}

void Jet::rollLeft( bool doit )
{
    m_btnRollLeft = doit;
}

void Jet::rollRight( bool doit )
{
    m_btnRollRight = doit;
}

void Jet::yawLeft( bool doit )
{
    m_btnYawLeft = doit;
}

void Jet::yawRight( bool doit )
{
    m_btnYawRight = doit;
}

void Jet::pitchUp( bool doit )
{
    m_btnPitchUp = doit;
}

void Jet::pitchDown( bool doit )
{
    m_btnPitchDown = doit;
}

void Jet::speedUp( bool doit )
{
    m_speedAcc += doit ? 1 : -1;
}

void Jet::speedDown( bool doit )
{
    speedUp( !doit );
}

bool Jet::isShooting( uint32_t weaponNum ) const
{
    return m_shooting[ weaponNum ];
}

void Jet::shoot( uint32_t weaponNum, bool doit )
{
    m_shooting[ weaponNum ] = doit;
}

Vertex Jet::weaponPoint( uint32_t weaponNum )
{
    Vertex w = m_model.weapon( weaponNum );
    m_quaternion.rotateVector( w );
    w = w + position();
    return w;
}

Bullet* Jet::weapon( uint32_t weaponNum )
{
    BulletProto tmp = m_weapon[ weaponNum ];
    Vertex w = m_model.weapon( weaponNum );
    m_quaternion.rotateVector( w );
    w = w + position();

    tmp.x = w.x;
    tmp.y = w.y;
    tmp.z = w.z;

    Bullet* b = new Bullet( tmp );
    b->setDirection( direction() );
    if ( tmp.type == Bullet::Type::eTorpedo ) {
        if ( m_target ) {
            if ( m_target->status() == Status::eAlive ) {
                b->setTarget( m_target );
            }
        }
    }
    return b;
}

void Jet::setWeapon( BulletProto bp, uint32_t id )
{
    m_weapon[ id ] = bp;
}

bool Jet::isWeaponReady( uint32_t weaponNum ) const
{
    return ( m_shotFactor[ weaponNum ] >= m_weapon[ weaponNum ].delay )
        && ( m_energy >= m_weapon[ weaponNum ].energy );
}

void Jet::takeEnergy( uint32_t weaponNum )
{
    m_energy -= m_weapon[ weaponNum ].energy;
    m_shotFactor[ weaponNum ] = 0;
}

void Jet::drawWireframe()
{
    m_model.drawWireframe();
}

void Jet::processCollision( std::vector<Bullet*>& bullets )
{
    if ( status() == Status::eDead ) {
        return;
    }
    for ( Bullet* it : bullets ) {
        if ( it->status() != Status::eAlive ) {
            continue;
        }
        if ( distanceV( position(), it->position() ) > 0.1 ) {
            continue;
        }
        setDamage( it->damage() );
        it->kill();
        if ( health() <= 0 ) {
            return;
        }
    }
}

void Jet::processCollision( SAObject* )
{
    assert( !"shall not be called" );
};

void Jet::addScore( int32_t s, bool b )
{
    if ( b ) {
        m_score += s;
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
