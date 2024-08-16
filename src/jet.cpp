#include "jet.hpp"

#include "autoaim.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "game_pipeline.hpp"

#include <algorithm>
#include <cassert>

static const math::vec3 defaultPyrLimits{ 80.0_deg, 40.0_deg, 120.0_deg };
static const math::vec3 defaultPyrSpeed{ 60.0_deg, 40.0_deg, 200.0_deg };

static math::vec3 pointMult( uint8_t a, uint8_t b, uint8_t c )
{
    return math::vec3{
        SAObject::pointsToMultiplier( a ),
        SAObject::pointsToMultiplier( b ),
        SAObject::pointsToMultiplier( c )
    };
};

Jet::Jet( const CreateInfo& ci ) noexcept
: m_thruster{ { ci.modelScale, ci.modelScale }, { ci.modelScale, ci.modelScale } }
, m_model{ ci.model }
, m_weapons{ ci.weapons }
, m_pyrLimits{ defaultPyrLimits }
, m_angleState{ {}, {}, defaultPyrSpeed * pointMult( ci.points.pitch, ci.points.yaw, ci.points.roll ) }
, m_points{ ci.points }
{
    m_direction.z = -1;
    m_health = static_cast<uint8_t>( 100.0f * pointsToMultiplier( m_points.hp ) );
    m_speedCurveMax *= pointsToMultiplier( m_points.speedMax );
    m_speed = math::curve( m_speedCurveMin, m_speedCurveNorm, m_speedCurveMax, 0.5f );
    m_speedTarget = { m_speed, m_speed, m_accell };
    setStatus( Status::eAlive );

    std::transform( m_weapons.begin(), m_weapons.end(), m_weaponsCooldown.begin(),
        [](const auto& w ) { return WeaponCooldown{ .ready = w.delay, }; } );

};

void Jet::render( RenderContext rctx ) const
{
    auto model = rctx.model;
    rctx.model = math::translate( rctx.model, position() );
    rctx.model *= math::toMat4( quat() );

    m_model.render( rctx );
    auto thrusters = m_model.thrusters();
    for ( const math::vec3& it : thrusters ) {
        m_thruster[ 0 ].renderAt( rctx, it );
    }

    PushData bd{
        .m_pipeline = g_pipelines[ Pipeline::eBeamBlob ],
        .m_verticeCount = 12,
        .m_instanceCount = 0,
    };
    PushConstant<Pipeline::eBeamBlob> bc{
        .m_model = model,
        .m_view = rctx.view,
        .m_projection = rctx.projection,
    };
    for ( uint32_t i = 0; i < MAX_SUPPORTED_WEAPON_COUNT; ++i ) {
        if ( m_weapons[ i ].type != Bullet::Type::eLaser ) continue;
        if ( !isShooting( i ) ) continue;
        bc.m_beams[ bd.m_instanceCount++ ] = PushConstant<Pipeline::eBeamBlob>::Beam{
            .m_position = weaponPoint( i ),
            .m_quat = quat(),
            .m_displacement{ 1.0_m, 1.0_m, 1000.0_m },
            .m_color1 = m_weapons[ i ].color1,
            .m_color2 = m_weapons[ i ].color2,
        };
    }
    if ( bd.m_instanceCount ) {
        rctx.renderer->push( bd, &bc );
    }
}

void Jet::update( const UpdateContext& updateContext )
{
    SAObject::update( updateContext );
    if ( status() == Status::eDead ) {
        return;
    }

    const math::vec3 pyrControl{ m_input.pitch, m_input.yaw, m_input.roll };
    const math::vec3 pyrTarget = pyrControl * m_pyrLimits;
    m_angleState.setTarget( pyrTarget );
    m_angleState.update( updateContext.deltaTime );

    {
        const float speedPos = ( 1.0f + m_input.speed ) * 0.5f;
        m_speedTarget.setTarget( math::curve( m_speedCurveMin, m_speedCurveNorm, m_speedCurveMax, speedPos ) );

        const uint8_t accell = m_input.speed > 0.01f;
        const uint8_t deaccell = m_input.speed < -0.01f;
        const uint8_t accellPoints[ 3 ]{ 0, m_points.accell, m_points.deaccell };
        const float accellMultiplier = pointsToMultiplier( accellPoints[ accell + deaccell * 2 ] );
        m_speedTarget.setVelocity( ( accell ? m_accell : m_deaccell ) * accellMultiplier );
        m_speedTarget.update( updateContext.deltaTime + updateContext.deltaTime * updateContext.deltaTime );
        m_speed = m_speedTarget.value();
    }

    m_quaternion *= math::quat{ m_angleState.value() * updateContext.deltaTime };
    m_direction = math::normalize( math::rotate( m_quaternion, math::vec3{ 0.0f, 0.0f, -1.0f } ) );


    auto nlerpRescale = []( float min, float max, float n )
    {
        return math::nonlerp( min, max, n * 0.5f + 0.5f );
    };

    m_camOffset.setTarget( math::vec3{
        nlerpRescale( 6.0_m, -6.0_m, m_input.yaw ),
        nlerpRescale( -2.0_m, 2.0_m, m_input.pitch ),
        0.0f }
    );
    m_camOffset.update( updateContext.deltaTime );

    m_thrusterLength.update( updateContext.deltaTime );
    math::vec2 thrusterLength = m_thrusterLength.value();
    m_thruster[ 0 ].setLength( thrusterLength.x );
    m_thruster[ 1 ].setLength( thrusterLength.y );

    for ( auto& wc : m_weaponsCooldown ) {
        wc.current = std::min( wc.current + updateContext.deltaTime, wc.ready );
    }

    m_position += velocity() * updateContext.deltaTime;
    m_thruster[ 0 ].update( updateContext );
    m_thruster[ 1 ].update( updateContext );

}

void Jet::scanSignals( std::span<const Signal> signals, float dt )
{
    auto tgt = SAObject::scanSignals( m_targetSignal.position, signals );
    if ( !tgt ) return;

    m_targetVelocity = ( tgt->position - m_targetSignal.position ) / dt;
    m_targetSignal = *tgt;
}

void Jet::setTarget( Signal v )
{
    m_targetSignal = v;
    m_targetVelocity = {};
}

math::vec4 Jet::targetingState() const
{
    const math::vec3 p = m_targetSignal.position;
    const float f = static_cast<float>( AutoAim{}.matches( position(), direction(), p ) );
    return math::vec4{ p, f };
}

bool Jet::isShooting( uint32_t weaponNum ) const
{
    switch ( weaponNum ) {
    case 0: return m_input.shoot1;
    case 1: return m_input.shoot2;
    case 2: return m_input.shoot3;
    [[unlikely]]
    default:
        assert( !"weapon index out of range" );
        return false;
    }
}

math::vec3 Jet::weaponPoint( uint32_t weaponNum ) const
{
    math::vec3 w = math::rotate( quat(), m_model.weapon( weaponNum ) );
    w += position();
    return w;
}

Bullet Jet::weapon( uint32_t weaponNum )
{
    assert( weaponNum < std::size( m_weapons ) );
    m_weaponsCooldown[ weaponNum ].current = 0.0f;
    Bullet b{ m_weapons[ weaponNum ], math::rotate( quat(), m_model.weapon( weaponNum ) ) + position(), direction() };
    b.m_collideId = COLLIDE_ID;
    switch ( b.m_type ) {
    case Bullet::Type::eLaser:
    case Bullet::Type::eTorpedo:
        break;
    case Bullet::Type::eBlaster: {
        AutoAim autoAim{};
        if ( !autoAim.matches( position(), direction(), m_targetSignal.position ) ) {
            break;
        }
        b.m_direction = autoAim( b.m_speed, b.m_position, m_targetSignal.position, m_targetVelocity );
    } break;
    default:
        assert( !"unreachable" );
        break;
    }
    b.m_quat = math::quatLookAt( b.m_direction, math::rotate( quat(), math::vec3{ 0.0f, 1.0f, 0.0f } ) );
    return b;
}

std::array<Bullet::Type, Jet::MAX_SUPPORTED_WEAPON_COUNT> Jet::shoot( std::pmr::vector<Bullet>& vec )
{
    std::array<Bullet::Type, MAX_SUPPORTED_WEAPON_COUNT> ret;
    std::fill( ret.begin(), ret.end(), Bullet::Type::eDead );

    for ( auto i = 0u; i < MAX_SUPPORTED_WEAPON_COUNT; ++i ) {
        const auto& wc = m_weaponsCooldown[ i ];
        if ( wc.current < wc.ready ) continue;
        if ( !isShooting( i ) ) continue;
        vec.emplace_back( weapon( i ) );
        ret[ i ] = m_weapons[ i ].type;
    }
    return ret;
}

math::quat Jet::quat() const
{
    return m_quaternion;
}

math::quat Jet::rotation() const
{
    return math::inverse( m_quaternion );
}

void Jet::setInput( const Jet::Input& input )
{
    m_input = input;
}

math::vec3 Jet::cameraPosition() const
{
    return m_camOffset.value() + math::vec3{ 0.0f, -10.5_m, 41.5_m };
}

math::vec3 Jet::cameraDirection() const
{
    return m_direction;
}
