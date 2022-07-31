#include "jet.hpp"

#include "autoaim.hpp"
#include "texture.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>

constexpr static math::vec3 defaultPyrLimits{ 80.0_deg, 40.0_deg, 120.0_deg };
constexpr static math::vec3 defaultPyrAnimLimits{ 5.0_deg, 5.0_deg, 15.0_deg };

Jet::Jet( const ModelProto& modelData ) noexcept
: m_thruster{ { modelData.scale, modelData.scale * 0.04285f }, { modelData.scale, modelData.scale * 0.04285f } }
, m_pyrLimits{ defaultPyrLimits }
{
    m_direction.z = -1;
    m_health = 100;
    m_speed = 600_kmph;
    setStatus( Status::eAlive );

    m_model = modelData.model;
};

void Jet::render( RenderContext rctx ) const
{
    rctx.model = math::translate( rctx.model, position() );
    rctx.model *= math::toMat4( quat() );
    rctx.model *= math::toMat4( math::quat{ m_animationAngleState.value() } );

    m_model.render( rctx );
    auto thrusters = m_model.thrusters();
    if ( !m_vectorThrust || thrusters.size() != 2 ) {
        for ( const math::vec3& it : thrusters ) {
            m_thruster[ 0 ].renderAt( rctx, it );
        }
        return;
    }
    const math::mat4 model = rctx.model;
    const math::vec2 left = m_vectorThrustLeft.value();
    const math::vec2 right = m_vectorThrustRight.value();

    rctx.model = math::rotate( model, right.x + right.y, axis::x );
    m_thruster[ 1 ].renderAt( rctx, thrusters[ 1 ] );

    rctx.model = math::rotate( model, left.x + left.y, axis::x );
    m_thruster[ 0 ].renderAt( rctx, thrusters[ 0 ] );
}

void Jet::update( const UpdateContext& updateContext )
{
    SAObject::update( updateContext );
    if ( status() == Status::eDead ) {
        return;
    }
    m_reactor.update( updateContext );

    const math::vec3 pyrControl{ m_input.pitch, m_input.yaw, m_input.roll };
    const math::vec3 pyrTarget = pyrControl * m_pyrLimits;
    m_angleState.setTarget( pyrTarget );
    m_angleState.update( updateContext.deltaTime );

    const math::vec3 pyrAnimTarget = pyrControl * defaultPyrAnimLimits;
    m_animationAngleState.setTarget( pyrAnimTarget );
    m_animationAngleState.update( updateContext.deltaTime );

    float speedTarget = m_input.speed >= 0
        ? std::lerp( m_speedNorm, m_speedMax, m_input.speed )
        : std::lerp( m_speedMin, m_speedNorm, 1.0f + m_input.speed );
    if ( m_input.speed > 0 ) {
        const float afterburnerCost = 70.0f * updateContext.deltaTime;
        m_reactor.consume( afterburnerCost );
    }

    m_speedTarget.setTarget( speedTarget );
    m_speedTarget.update( updateContext.deltaTime );
    m_speed = m_speedTarget.value();

    m_quaternion *= math::quat{ m_angleState.value() * updateContext.deltaTime };
    m_direction = math::normalize( math::rotate( m_quaternion, math::vec3{ 0.0f, 0.0f, -1.0f } ) );


    float horizontalN = m_input.yaw * 0.5f + 0.5f;
    float verticalN = m_input.pitch * 0.5f + 0.5f;
    float horizontalPos = math::lerp( 0.2f, -0.2f, horizontalN );
    float verticalPos = math::lerp( -0.2f, 0.2f, verticalN );
    float horizontalPos2 = math::lerp( 6.0_m, -6.0_m, horizontalN );
    float verticalPos2 = math::lerp( -2.0_m, 2.0_m, verticalN );
    m_camDirection.setTarget( math::normalize( math::vec3{ horizontalPos, verticalPos, -1.0f } ) );
    m_camDirection.update( updateContext.deltaTime );
    m_camPosition.setTarget( { horizontalPos2, verticalPos2, 0.0f } );
    m_camPosition.update( updateContext.deltaTime );

    m_vectorThrustLeft.setTarget( { -5.0_deg * m_input.pitch, 5.0_deg * m_input.roll } );
    m_vectorThrustRight.setTarget( { -5.0_deg * m_input.pitch, -5.0_deg * m_input.roll } );
    m_vectorThrustLeft.update( updateContext.deltaTime );
    m_vectorThrustRight.update( updateContext.deltaTime );

    float left = 10.0_m + ( m_vectorThrust ? m_input.yaw * 3.0_m : 0.0f ) + m_input.speed * 4.0_m;
    float right = 10.0_m + ( m_vectorThrust ? m_input.yaw * -3.0_m : 0.0f ) + m_input.speed * 4.0_m;
    m_thrusterLength.setTarget( { left, right } );
    m_thrusterLength.update( updateContext.deltaTime );
    math::vec2 thrusterLength = m_thrusterLength.value();
    m_thruster[ 0 ].setLength( thrusterLength.x );
    m_thruster[ 1 ].setLength( thrusterLength.y );

    for ( auto i : { 0, 1, 2 } ) {
        if ( m_weaponCooldown[ i ] >= m_weapon[ i ].delay ) continue;
        m_weaponCooldown[ i ] += updateContext.deltaTime;
    }

    m_position += velocity() * updateContext.deltaTime;
    m_thruster[ 0 ].update( updateContext );
    m_thruster[ 1 ].update( updateContext );

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
    math::vec3 w = math::rotate( quat(), m_model.weapon( weaponNum ) );
    w += position();
    return w;
}

UniquePointer<Bullet> Jet::weapon( uint32_t weaponNum, std::pmr::memory_resource* alloc )
{
    assert( weaponNum < std::size( m_weapon ) );
    assert( alloc );
    BulletProto tmp = m_weapon[ weaponNum ];
    tmp.position = math::rotate( quat(), m_model.weapon( weaponNum ) ) + position();
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
    return ( m_weaponCooldown[ weaponNum ] >= m_weapon[ weaponNum ].delay )
        && ( (uint32_t)m_reactor.power() >= m_weapon[ weaponNum ].energy );
}

void Jet::takeEnergy( uint32_t weaponNum )
{
    m_reactor.consume( (float)m_weapon[ weaponNum ].energy );
    m_weaponCooldown[ weaponNum ] = 0.0f;
}

double Jet::energy() const
{
    return m_reactor.power();
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

math::vec3 Jet::cameraPosition() const
{
    return m_camPosition.value() + math::vec3{ 0.0f, -10.5_m, 41.5_m };
}

math::vec3 Jet::cameraDirection() const
{
    return m_direction;
}
