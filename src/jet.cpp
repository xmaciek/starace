#include "jet.hpp"

#include "texture.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

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

void Jet::render( RenderContext ) const
{
}

void Jet::draw() const
{
    glPushMatrix();
    const glm::mat4 matrice = glm::toMat4( animation() );
    glMultMatrixf( glm::value_ptr( matrice ) );

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

void Jet::update( const UpdateContext& updateContext )
{
    if ( m_btnRollLeft ) {
        if ( m_rotZ < m_maxAngleZ ) {
            m_rotZ += m_angleSpeedZ * updateContext.deltaTime;
        }
    }
    if ( m_btnRollRight ) {
        if ( m_rotZ > -m_maxAngleZ ) {
            m_rotZ -= m_angleSpeedZ * updateContext.deltaTime;
        }
    }
    if ( ( !m_btnRollLeft && !m_btnRollRight ) || ( m_btnRollLeft && m_btnRollRight ) ) {
        m_roll = 0;
        if ( m_rotZ < -0.1 * updateContext.deltaTime ) {
            m_rotZ += m_angleSpeedZ * updateContext.deltaTime;
        }
        if ( m_rotZ > 0.1 * updateContext.deltaTime ) {
            m_rotZ -= m_angleSpeedZ * updateContext.deltaTime;
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
            m_rotY += m_angleSpeedY * updateContext.deltaTime;
        }
    }
    if ( m_btnYawRight ) {
        if ( m_rotY > -m_maxAngleY ) {
            m_rotY -= m_angleSpeedY * updateContext.deltaTime;
        }
    }
    if ( ( !m_btnYawLeft && !m_btnYawRight ) || ( m_btnYawLeft && m_btnYawRight ) ) {
        m_yaw = 0;
        if ( m_rotY < -0.1 * updateContext.deltaTime ) {
            m_rotY += m_angleSpeedY * updateContext.deltaTime;
        }
        if ( m_rotY > 0.1 * updateContext.deltaTime ) {
            m_rotY -= m_angleSpeedY * updateContext.deltaTime;
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
            m_rotX += m_angleSpeedX * updateContext.deltaTime;
        }
    }
    if ( m_btnPitchDown ) {
        if ( m_rotX > -m_maxAngleX ) {
            m_rotX -= m_angleSpeedX * updateContext.deltaTime;
        }
    }
    if ( ( !m_btnPitchUp && !m_btnPitchDown ) || ( m_btnPitchUp && m_btnPitchDown ) ) {
        m_pitch = 0;
        if ( m_rotX < -0.1 * updateContext.deltaTime ) {
            m_rotX += m_angleSpeedX * updateContext.deltaTime;
        }
        if ( m_rotX > 0.1 * updateContext.deltaTime ) {
            m_rotX -= m_angleSpeedX * updateContext.deltaTime;
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
        m_speed += 0.8f * updateContext.deltaTime;
    }
    else {
        if ( ( m_speed > m_minSpeed ) && ( m_speedAcc < 0 ) ) {
            m_speed -= 0.8f * updateContext.deltaTime;
        }
    }
    if ( ( ( m_speed >= m_normSpeed + 0.1 * updateContext.deltaTime ) || ( speed() <= m_normSpeed - 0.1f * updateContext.deltaTime ) ) && ( m_speedAcc == 0 ) ) {
        if ( speed() < m_normSpeed ) {
            m_speed += 0.3f * updateContext.deltaTime;
        }
        else {
            m_speed -= 0.3f * updateContext.deltaTime;
        }
    }
    if ( speed() < 3.0f ) {
        m_thruster.setLength( speed() / 8.0f );
    }

    m_animation = glm::quat{ glm::vec3{ glm::radians( -m_rotX ), glm::radians( m_rotY ), glm::radians( m_rotZ ) } };
    m_quaternion = glm::conjugate( m_quaternion );
    m_quaternion = glm::rotate( m_quaternion, glm::radians( -m_rotX ) * 4.0f * updateContext.deltaTime, glm::vec3{ 1.0f, 0.0f, 0.0f } );
    m_quaternion = glm::rotate( m_quaternion, glm::radians( m_rotY ) * 2.5f * updateContext.deltaTime, glm::vec3{ 0.0f, 1.0f, 0.0f } );
    m_quaternion = glm::rotate( m_quaternion, glm::radians( m_rotZ ) * 2.5f * updateContext.deltaTime, glm::vec3{ 0.0f, 0.0f, 1.0f } );
    m_rotation = m_quaternion;
    m_quaternion = glm::conjugate( m_quaternion );

    m_direction = glm::normalize( glm::rotate( m_rotation, glm::vec3{ 0.0f, 0.0f, -1.0f } ) );
    m_velocity = direction() * speed();

    if ( m_shotFactor[ 0 ] < m_weapon[ 0 ].delay ) {
        m_shotFactor[ 0 ] += 1.0 * updateContext.deltaTime;
    }
    if ( m_shotFactor[ 1 ] < m_weapon[ 1 ].delay ) {
        m_shotFactor[ 1 ] += 1.0 * updateContext.deltaTime;
    }
    if ( m_shotFactor[ 2 ] < m_weapon[ 2 ].delay ) {
        m_shotFactor[ 2 ] += 1.0 * updateContext.deltaTime;
    }

    m_energy = std::min( m_energy + 60.0 * updateContext.deltaTime, 100.0 );

    m_position += velocity() * updateContext.deltaTime;
    m_thruster.update( updateContext );
    m_shield.update( updateContext );
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

glm::vec3 Jet::weaponPoint( uint32_t weaponNum )
{
    glm::vec3 w = glm::rotate( m_rotation, m_model.weapon( weaponNum ) );
    w += position();
    return w;
}

Bullet* Jet::weapon( uint32_t weaponNum )
{
    BulletProto tmp = m_weapon[ weaponNum ];
    glm::vec3 w = glm::rotate( m_rotation, m_model.weapon( weaponNum ) );
    w += position();

    tmp.position = w;

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
        if ( glm::distance( position(), it->position() ) > 0.1f ) {
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

glm::quat Jet::animation() const
{
    return m_animation;
}

glm::quat Jet::quat() const
{
    return m_quaternion;
}

glm::quat Jet::rotation() const
{
    return m_rotation;
}
