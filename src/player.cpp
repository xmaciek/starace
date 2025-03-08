#include "player.hpp"

#include "autoaim.hpp"
#include "colors.hpp"
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

Player::Player( const CreateInfo& ci ) noexcept
: m_model{ ci.model }
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
    for ( uint32_t i = 0; i < MAX_SUPPORTED_WEAPON_COUNT; ++i ) {
        m_weapons[ i ] = ci.weapons[ i ];
    }

};

void Player::render( RenderContext rctx ) const
{
    auto model = rctx.model;
    rctx.model = math::translate( rctx.model, position() );
    rctx.model *= math::toMat4( quat() );

    m_model.render( rctx );

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
        if ( m_weapons[ i ].m_ci.type != Bullet::Type::eLaser ) continue;
        if ( !isShooting( i ) ) continue;
        bc.m_beams[ bd.m_instanceCount++ ] = PushConstant<Pipeline::eBeamBlob>::Beam{
            .m_position = weaponPoint( i ),
            .m_quat = quat(),
            .m_displacement{ 1.0_m, 1.0_m, 1000.0_m },
            .m_color1 = color::crimson,
            .m_color2 = color::white,
        };
    }
    if ( bd.m_instanceCount ) {
        rctx.renderer->push( bd, &bc );
    }
}

void Player::update( const UpdateContext& uctx )
{
    m_health -= std::min( m_health, m_pendingDamage );
    m_pendingDamage = 0;
    if ( m_health == 0 ) {
        m_status = Status::eDead;
        return;
    }

    scanSignals( uctx.signals, uctx.deltaTime );
    const math::vec3 pyrControl{ m_input.pitch, m_input.yaw, m_input.roll };
    const math::vec3 pyrTarget = pyrControl * m_pyrLimits;
    m_angleState.setTarget( pyrTarget );
    m_angleState.update( uctx.deltaTime );

    {
        const float speedPos = ( 1.0f + m_input.speed ) * 0.5f;
        m_speedTarget.setTarget( math::curve( m_speedCurveMin, m_speedCurveNorm, m_speedCurveMax, speedPos ) );

        const uint8_t accell = m_input.speed > 0.01f;
        const uint8_t deaccell = m_input.speed < -0.01f;
        const uint8_t accellPoints[ 3 ]{ 0, m_points.accell, m_points.deaccell };
        const float accellMultiplier = pointsToMultiplier( accellPoints[ accell + deaccell * 2 ] );
        m_speedTarget.setVelocity( ( accell ? m_accell : m_deaccell ) * accellMultiplier );
        m_speedTarget.update( uctx.deltaTime + uctx.deltaTime * uctx.deltaTime );
        m_speed = m_speedTarget.value();
    }

    m_quaternion *= math::quat{ m_angleState.value() * uctx.deltaTime };
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
    m_camOffset.update( uctx.deltaTime );

    std::ranges::for_each( m_weapons, [&uctx]( auto& w ) { w.update( uctx ); } );

    m_position += velocity() * uctx.deltaTime;
}

void Player::scanSignals( std::span<const Signal> signals, float dt )
{
    auto tgt = SAObject::scanSignals( m_targetSignal.position, signals );
    if ( !tgt ) return;

    m_targetVelocity = ( tgt.position - m_targetSignal.position ) / dt;
    m_targetSignal = tgt;
}

void Player::setTarget( Signal v )
{
    m_targetSignal = v;
    m_targetVelocity = {};
}

float Player::targetingState() const
{
    const math::vec3 p = m_targetSignal.position;
    return static_cast<float>( AutoAim{}.matches( position(), direction(), p ) );
}

bool Player::isShooting( uint32_t weaponNum ) const
{
    switch ( weaponNum ) {
    case 0: return m_input.shoot1;
    case 1: return m_input.shoot2;
    case 2: return m_input.shoot1;
    [[unlikely]]
    default:
        assert( !"weapon index out of range" );
        return false;
    }
}

math::vec3 Player::weaponPoint( uint32_t weaponNum ) const
{
    math::vec3 w = math::rotate( quat(), m_model.weapon( weaponNum ) );
    w += position();
    return w;
}

Bullet Player::weapon( uint32_t weaponNum )
{
    assert( weaponNum < std::size( m_weapons ) );
    AutoAim aa{};
    WeaponCreateInfo wci = m_weapons[ weaponNum ].fire();
    math::vec3 projectilePosition = math::rotate( quat(), m_model.weapon( weaponNum ) ) + position();
    math::vec3 projectileDirection = wci.type == Bullet::Type::eBlaster && aa.matches( position(), direction(), m_targetSignal.position )
        ? aa( wci.speed, projectilePosition, m_targetSignal.position, m_targetVelocity )
        : direction();

    Bullet b{ wci, projectilePosition, projectileDirection };
    b.m_collideId = COLLIDE_ID;
    b.m_target = m_targetSignal;
    b.m_quat = math::quatLookAt( b.m_direction, math::rotate( quat(), math::vec3{ 0.0f, 1.0f, 0.0f } ) );
    return b;
}

std::array<Audio::Slot, Player::MAX_SUPPORTED_WEAPON_COUNT> Player::shoot( std::pmr::vector<Bullet>& vec )
{
    std::array<Audio::Slot, MAX_SUPPORTED_WEAPON_COUNT> ret{};
    for ( auto i = 0u; i < MAX_SUPPORTED_WEAPON_COUNT; ++i ) {
        const auto& wc = m_weapons[ i ];
        if ( !wc.ready() ) continue;
        if ( !isShooting( i ) ) continue;
        vec.emplace_back( weapon( i ) );
        ret[ i ] = m_weapons[ i ].m_ci.sound;
    }
    return ret;
}

math::quat Player::quat() const
{
    return m_quaternion;
}

math::quat Player::rotation() const
{
    return math::inverse( m_quaternion );
}

void Player::setInput( const Player::Input& input )
{
    m_input = input;
}

math::vec3 Player::cameraPosition() const
{
    return m_camOffset.value() + math::vec3{ 0.0f, -10.5_m, 41.5_m };
}

math::vec3 Player::cameraDirection() const
{
    return m_direction;
}

math::vec2 Player::reloadState() const
{
    return math::vec2{
        m_weapons[ 0 ].m_reload / m_weapons[ 0 ].m_ci.reload,
        m_weapons[ 1 ].m_reload / m_weapons[ 1 ].m_ci.reload
    };
}

std::tuple<uint32_t, uint32_t> Player::weaponClip() const
{
    return std::make_tuple( m_weapons[ 0 ].m_count, m_weapons[ 1 ].m_count );
}
