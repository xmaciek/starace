#include <input/remapper.hpp>

#include <extra/fnta.hpp>
#include <unicode/unicode.hpp>

#include <cassert>
#include <algorithm>
#include <string_view>

#include <SDL_keyboard.h>


namespace input {

static constexpr char32_t XBOX_REMAP_OFFSET = 32;
static constexpr char32_t PS4_REMAP_OFFSET = 64;

static char32_t mapController( Actuator a, char32_t remapOffset )
{
    switch ( a.typed.type ) {
    case Actuator::BUTTONCODE:
        switch ( a.typed.id ) {
        case SDL_GAMEPAD_BUTTON_SOUTH: return remapOffset + fnta::Input::A;
        case SDL_GAMEPAD_BUTTON_EAST: return remapOffset + fnta::Input::B;
        case SDL_GAMEPAD_BUTTON_WEST: return remapOffset + fnta::Input::X;
        case SDL_GAMEPAD_BUTTON_NORTH: return remapOffset + fnta::Input::Y;
        case SDL_GAMEPAD_BUTTON_BACK: return remapOffset + fnta::Input::Select;
        case SDL_GAMEPAD_BUTTON_START: return remapOffset + fnta::Input::Start;
        case SDL_GAMEPAD_BUTTON_LEFT_STICK: return remapOffset + fnta::Input::L;
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK: return remapOffset + fnta::Input::R;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER: return remapOffset + fnta::Input::LB;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return remapOffset + fnta::Input::RB;
        case SDL_GAMEPAD_BUTTON_DPAD_UP: return remapOffset + fnta::Input::Up;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN: return remapOffset + fnta::Input::Down;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT: return remapOffset + fnta::Input::Left;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT: return remapOffset + fnta::Input::Right;
        default: assert( !"unhandled button enum" ); return 0;
        }

    case Actuator::AXISCODE:
        switch ( a.typed.id ) {
        case SDL_GAMEPAD_AXIS_LEFTX: return remapOffset + fnta::Input::LX;
        case SDL_GAMEPAD_AXIS_LEFTY: return remapOffset + fnta::Input::LY;
        case SDL_GAMEPAD_AXIS_RIGHTX: return remapOffset + fnta::Input::RX;
        case SDL_GAMEPAD_AXIS_RIGHTY: return remapOffset + fnta::Input::RY;
        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER: return remapOffset + fnta::Input::LT;
        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return remapOffset + fnta::Input::RT;
        default: assert( !"unhandled axis enum" ); return 0;
        }

    default:
        assert( !"faulty data in actuator" );
        return 0;
    }
}

Remapper::Vector Remapper::apply( Actuator::Source source, char32_t chr ) const
{
    const bool isController = source == Actuator::Source::eKBM;
    auto it = std::ranges::find_if( m_data, [chr, isController]( const auto& p )
    {
        if ( (char32_t)p.m_userEnum != chr ) return false;
        const bool c = p.m_max.source == Actuator::Source::eKBM;
        return c == isController;
    } );
    if ( it == m_data.end() ) return {};

    using Source = Actuator::Source;
    Vector ret;
    switch ( source ) {
    case Source::eKBM: {
        const SDL_Keycode vk = SDL_GetKeyFromScancode( (SDL_Scancode)it->m_max.typed.id, SDL_KMOD_NONE, false );
        const char* keyName = SDL_GetKeyName( vk );
        if ( !keyName ) return ret;


        std::string_view sv{ keyName };
        assert( sv.size() + 2 < ret.capacity() );
        unicode::Transcoder transcoder{ sv };
        ret.emplace_back( U'[' );
        ret.resize( 1u + transcoder.length() );
        std::generate_n( ret.begin() + 1u, transcoder.length(), transcoder );
        ret.emplace_back( U']' );
        return ret;
    }
    case Source::ePS4:
        if ( auto c = mapController( it->m_max, PS4_REMAP_OFFSET ) ) ret.emplace_back( c );
        return ret;
    case Source::eXBoxOne:
        if ( auto c = mapController( it->m_max, XBOX_REMAP_OFFSET ) ) ret.emplace_back( c );
        return ret;
    default: assert( !"unhandled enum" );
    }
    return ret;
}

void Remapper::add( Action::Enum e, Actuator a )
{
    m_data.push_back( Pair{ .m_max = a, .m_userEnum = e } );
}

void Remapper::add( Action::Enum e, Actuator a1, Actuator a2 )
{
    m_data.push_back( Pair{ .m_min = a1, .m_max = a2, .m_userEnum = e } );
}

std::pmr::vector<Action> Remapper::updateAndResolve( Actuator a )
{
    std::pmr::vector<Action> ret;
    for ( auto& it : m_data ) {
        if ( it.m_max == a ) {
            it.m_maxF = a.value;
            ret.emplace_back( it.makeAction() );
        }
        else if ( it.m_min == a ) {
            it.m_minF = a.value;
            ret.emplace_back( it.makeAction() );
        }
    }
    return ret;
}

Action Remapper::Pair::makeAction() const
{
    int32_t vmax = m_maxF;
    int32_t vmin = -(int32_t)m_minF;
    int32_t value = std::clamp<int32_t>( vmax + vmin, Actuator::MIN, Actuator::MAX );
    return Action{ .userEnum = m_userEnum, .value = static_cast<Actuator::value_type>( value ) };
}






}
