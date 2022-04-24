#include "jet.hpp"

#include "autoaim.hpp"
#include "texture.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>

constexpr static math::vec3 defaultPyrAcceleration{ 30.0_deg, 20.0_deg, 100.0_deg };
constexpr static math::vec3 defaultPyrLimits{ 80.0_deg, 40.0_deg, 120.0_deg };
constexpr static math::vec3 defaultPyrAnimLimits{ 5.0_deg, 5.0_deg, 15.0_deg };

Jet::Jet( const ModelProto& modelData ) noexcept
: m_thruster( modelData.scale, (float)modelData.scale * 0.04285f )
, m_shield( 0.15, 0.03 )
, m_pyrAccelleration{ defaultPyrAcceleration }
, m_pyrLimits{ defaultPyrLimits }
{
    m_direction.z = -1;
    m_health = 100;
    m_speed = 4;
    setStatus( Status::eAlive );

    m_model = modelData.model;
};

void Jet::render( RenderContext rctx ) const
{
    rctx.model = math::translate( rctx.model, position() );
    rctx.model *= math::toMat4( quat() );
    rctx.model *= math::toMat4( animation() );

    m_model->render( rctx );
    for ( const math::vec3& it : m_model->thrusters() ) {
        m_thruster.renderAt( rctx, it );
    }
}

void Jet::update( const UpdateContext& updateContext )
{
    SAObject::update( updateContext );
    if ( status() == Status::eDead ) {
        return;
    }
    m_reactor.update( updateContext );

    const math::vec3 pyrControl{ m_input.pitch, m_input.yaw, m_input.roll };
    m_pyrTarget = pyrControl * m_pyrLimits;
    const math::vec3 dir = m_pyrTarget - m_pyrCurrent;
    if ( const float length = math::length( dir ); length > 0.0f ) {
        const math::vec3 pyrAdd = math::normalize( dir ) * m_pyrAccelleration * updateContext.deltaTime;
        if ( length >= math::length( pyrAdd ) ) {
            m_pyrCurrent = math::clamp( m_pyrCurrent + pyrAdd, -m_pyrLimits, m_pyrLimits );
        }
        else {
            m_pyrCurrent = m_pyrTarget;
        }
    }

    {
        const math::vec3 aTarget = pyrControl * defaultPyrAnimLimits;
        const math::vec3 aDir = aTarget - m_pyrAnimCurrent;
        if ( const float length = math::length( aDir ); length > 0.0f ) {
            const math::vec3 animAdd = math::normalize( aDir ) * m_pyrAccelleration * updateContext.deltaTime;
            if ( length > math::length( animAdd ) ) {
                m_pyrAnimCurrent = math::clamp( m_pyrAnimCurrent + animAdd, -defaultPyrAnimLimits, defaultPyrAnimLimits  );
            }
            else {
                m_pyrAnimCurrent = aTarget;
            }
            const math::quat qx{ m_pyrAnimCurrent * math::vec3{ 1.0f, 0.0f, 0.0f } };
            const math::quat qy{ m_pyrAnimCurrent * math::vec3{ 0.0f, 1.0f, 0.0f } };
            const math::quat qz{ m_pyrAnimCurrent * math::vec3{ 0.0f, 0.0f, 1.0f } };
            m_animation = qz * ( qy * ( math::quat{ math::vec3{} } * qx ) );
        }
    }

    m_speedTarget = m_input.speed >= 0
        ? std::lerp( m_speedNorm, m_speedMax, m_input.speed )
        : std::lerp( m_speedMin, m_speedNorm, 1.0f + m_input.speed );
    if ( m_input.speed > 0 ) {
        const float afterburnerCost = 70.0f * updateContext.deltaTime;
        m_reactor.consume( afterburnerCost );
    }

    m_speed += math::clamp( m_speedTarget - m_speed, -m_speedAcceleration, m_speedAcceleration )
        * updateContext.deltaTime;

    m_quaternion *= math::quat{ m_pyrCurrent * updateContext.deltaTime };
    m_direction = math::normalize( math::rotate( m_quaternion, math::vec3{ 0.0f, 0.0f, -1.0f } ) );

    if ( speed() < 6.0f ) {
        m_thruster.setLength( speed() / 16.0f );
    }

    if ( m_shotFactor[ 0 ] < m_weapon[ 0 ].delay ) {
        m_shotFactor[ 0 ] += updateContext.deltaTime;
    }
    if ( m_shotFactor[ 1 ] < m_weapon[ 1 ].delay ) {
        m_shotFactor[ 1 ] += updateContext.deltaTime;
    }
    if ( m_shotFactor[ 2 ] < m_weapon[ 2 ].delay ) {
        m_shotFactor[ 2 ] += updateContext.deltaTime;
    }


    m_position += velocity() * updateContext.deltaTime;
    m_thruster.update( updateContext );
    m_shield.update( updateContext );
    if ( m_target ) {
        if ( m_target->status() != Status::eAlive ) {
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

math::vec3 Jet::weaponPoint( uint32_t weaponNum )
{
    math::vec3 w = math::rotate( quat(), m_model->weapon( weaponNum ) );
    w += position();
    return w;
}

UniquePointer<Bullet> Jet::weapon( uint32_t weaponNum, std::pmr::memory_resource* alloc )
{
    assert( weaponNum < std::size( m_weapon ) );
    assert( alloc );
    BulletProto tmp = m_weapon[ weaponNum ];
    tmp.position = math::rotate( quat(), m_model->weapon( weaponNum ) ) + position();
    UniquePointer<Bullet> b{ alloc, tmp };
    assert( b->status() != Status::eDead );
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
        AutoAim autoAim{};
        if ( !autoAim.matches( position(), direction(), m_target->position() ) ) {
            break;
        }
        b->setDirection( autoAim( b->speed(), b->position(), m_target->position(), m_target->velocity() ) );
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

double Jet::energy() const
{
    return m_reactor.power();
}

math::quat Jet::animation() const
{
    return m_animation;
}

math::quat Jet::quat() const
{
    return m_quaternion;
}

math::quat Jet::rotation() const
{
    return math::inverse( m_quaternion );
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
