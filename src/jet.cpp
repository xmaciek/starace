#include "jet.hpp"

#include "texture.hpp"
#include "utils.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <cassert>

constexpr static float operator ""_deg ( long double f ) noexcept
{
    return glm::radians( static_cast<float>( f ) );
}

constexpr static glm::vec3 defaultPyrAcceleration{ 15.0_deg, 10.0_deg, 50.0_deg };
constexpr static glm::vec3 defaultPyrLimits{ 40.0_deg, 20.0_deg, 60.0_deg };
constexpr static glm::vec3 defaultPyrAnimLimits{ 5.0_deg, 5.0_deg, 15.0_deg };

Jet::Jet( const ModelProto& modelData )
: m_thruster( modelData.scale, (float)modelData.scale * 0.04285f )
, m_shield( 0.15, 0.03 )
, m_pyrAccelleration{ defaultPyrAcceleration }
, m_pyrLimits{ defaultPyrLimits }
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

    m_thruster.setColorScheme( colorscheme::ion );
};

void Jet::render( RenderContext rctx ) const
{
    rctx.model = glm::translate( rctx.model, position() );
    rctx.model *= glm::toMat4( quat() );
    rctx.model *= glm::toMat4( animation() );

    m_model.render( rctx );
    for ( const glm::vec3& it : m_model.thrusters() ) {
        m_thruster.renderAt( rctx, it );
    }
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
    SAObject::update( updateContext );
    if ( status() == Status::eDead ) {
        return;
    }
    m_reactor.update( updateContext );

    const glm::vec3 pyrControl{ m_input.pitch, m_input.yaw, m_input.roll };
    m_pyrTarget = pyrControl * m_pyrLimits;
    const glm::vec3 dir = m_pyrTarget - m_pyrCurrent;
    if ( const float length = glm::length( dir ); length > 0.0f ) {
        const glm::vec3 pyrAdd = glm::normalize( dir ) * m_pyrAccelleration * updateContext.deltaTime;
        if ( length >= glm::length( pyrAdd ) ) {
            m_pyrCurrent = glm::clamp( m_pyrCurrent + pyrAdd, -m_pyrLimits, m_pyrLimits );
        }
        else {
            m_pyrCurrent = m_pyrTarget;
        }
    }

    {
        const glm::vec3 aTarget = pyrControl * defaultPyrAnimLimits;
        const glm::vec3 aDir = aTarget - m_pyrAnimCurrent;
        if ( const float length = glm::length( aDir ); length > 0.0f ) {
            const glm::vec3 animAdd = glm::normalize( aDir ) * m_pyrAccelleration * updateContext.deltaTime;
            if ( length > glm::length( animAdd ) ) {
                m_pyrAnimCurrent = glm::clamp( m_pyrAnimCurrent + animAdd, -defaultPyrAnimLimits, defaultPyrAnimLimits  );
            }
            else {
                m_pyrAnimCurrent = aTarget;
            }
            const glm::quat qx{ m_pyrAnimCurrent * glm::vec3{ 1.0f, 0.0f, 0.0f } };
            const glm::quat qy{ m_pyrAnimCurrent * glm::vec3{ 0.0f, 1.0f, 0.0f } };
            const glm::quat qz{ m_pyrAnimCurrent * glm::vec3{ 0.0f, 0.0f, 1.0f } };
            m_animation = qz * ( qy * ( glm::quat{ glm::vec3{} } * qx ) );
        }
    }

    m_speedTarget = m_input.speed >= 0
        ? lerp( m_speedNorm, m_speedMax, m_input.speed )
        : lerp( m_speedMin, m_speedNorm, 1.0f + m_input.speed );
    if ( m_input.speed > 0 ) {
        const float afterburnerCost = 70.0f * updateContext.deltaTime;
        m_reactor.consume( afterburnerCost );
    }

    m_speed += glm::clamp( m_speedTarget - m_speed, -m_speedAcceleration, m_speedAcceleration )
        * updateContext.deltaTime;

    m_quaternion *= glm::quat{ m_pyrCurrent * updateContext.deltaTime };
    m_direction = glm::normalize( glm::rotate( m_quaternion, glm::vec3{ 0.0f, 0.0f, -1.0f } ) );
    m_velocity = direction() * speed();

    if ( speed() < 3.0f ) {
        m_thruster.setLength( speed() / 8.0f );
    }

    if ( m_shotFactor[ 0 ] < m_weapon[ 0 ].delay ) {
        m_shotFactor[ 0 ] += 1.0f * updateContext.deltaTime;
    }
    if ( m_shotFactor[ 1 ] < m_weapon[ 1 ].delay ) {
        m_shotFactor[ 1 ] += 1.0f * updateContext.deltaTime;
    }
    if ( m_shotFactor[ 2 ] < m_weapon[ 2 ].delay ) {
        m_shotFactor[ 2 ] += 1.0f * updateContext.deltaTime;
    }


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

bool Jet::isShooting( uint32_t weaponNum ) const
{
    switch ( weaponNum ) {
    case 0: return m_input.shoot1;
    case 1: return m_input.shoot2;
    case 2: return m_input.shoot3;
    default: return false;
    }
}

glm::vec3 Jet::weaponPoint( uint32_t weaponNum )
{
    glm::vec3 w = glm::rotate( quat(), m_model.weapon( weaponNum ) );
    w += position();
    return w;
}

Bullet* Jet::weapon( uint32_t weaponNum, void* ptr )
{
    assert( ptr );
    BulletProto tmp = m_weapon[ weaponNum ];
    tmp.position = glm::rotate( quat(), m_model.weapon( weaponNum ) ) + position();
    Bullet* b = new ( ptr ) Bullet( tmp );
    b->setDirection( direction() );

    switch ( b->type() ) {
    case Bullet::Type::eTorpedo:
        if ( m_target && m_target->status() == Status::eAlive ) {
            b->setTarget( m_target );
        }
        break;
    case Bullet::Type::eBlaster: {
        // auto-aiming
        if ( !m_target || m_target->status() != Status::eAlive ) {
            break;
        }
        const glm::vec3 tgtPos = m_target->position();
        {
            const glm::vec3 dir = direction();
            const glm::vec3 dirToTgt = glm::normalize( tgtPos - position() );
            const float angleToTarget = glm::abs( glm::acos(
                glm::dot( dir, dirToTgt )
                / ( glm::length( dir ) * glm::length( dirToTgt ) )
            ) );
            constexpr float a = glm::radians( 15.0f );
            if ( angleToTarget > a ) {
                break;
            }
        }
        const glm::vec3 tgtVelocity = m_target->velocity();
        const glm::vec3 bPos = b->position();
        const float bSpeed = b->speed();
        glm::vec3 tgtMaybePos = tgtPos;
        // 6 iterations is perfect enough
        for ( int i = 0; i < 10; ++i ) {
            const glm::vec3 distance = tgtMaybePos - bPos;
            const float time = glm::length( distance ) / bSpeed;
            tgtMaybePos = tgtPos + tgtVelocity * time;
        }
        b->setDirection( glm::normalize( tgtMaybePos - bPos ) );
    } break;
    default:
        break;
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
        && ( (uint32_t)m_reactor.power() >= m_weapon[ weaponNum ].energy );
}

void Jet::takeEnergy( uint32_t weaponNum )
{
    m_reactor.consume( (float)m_weapon[ weaponNum ].energy );
    m_shotFactor[ weaponNum ] = 0;
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

void Jet::addScore( uint32_t s, bool b )
{
    if ( b ) {
        m_score += s;
    }
}

double Jet::energy() const
{
    return m_reactor.power();
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
    return glm::inverse( m_quaternion );
}

void Jet::untarget( const SAObject* tgt )
{
    if ( m_target == tgt ) {
        m_target = nullptr;
    }
}

void Jet::setInput( const Jet::Input& input )
{
    m_input = input;
}
