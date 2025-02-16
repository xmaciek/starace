#include <input/remapper.hpp>

#include <extra/fnta.hpp>

#include <cassert>
#include <algorithm>
#include <string_view>

#include <SDL_keyboard.h>


namespace input {

static constexpr uint32_t XBOX_REMAP_OFFSET = 32;

static uint32_t mapController( Actuator a, uint32_t remapOffset, std::span<char32_t> out )
{
    switch ( a.typed.id ) {
    case SDL_CONTROLLER_BUTTON_A: *out.begin() = remapOffset + fnta::Input::A; return 1;
    case SDL_CONTROLLER_BUTTON_B: *out.begin() = remapOffset + fnta::Input::B; return 1;
    case SDL_CONTROLLER_BUTTON_X: *out.begin() = remapOffset + fnta::Input::X; return 1;
    case SDL_CONTROLLER_BUTTON_Y: *out.begin() = remapOffset + fnta::Input::Y; return 1;
    case SDL_CONTROLLER_BUTTON_BACK: *out.begin() = remapOffset + fnta::Input::Select; return 1;
    case SDL_CONTROLLER_BUTTON_START: *out.begin() = remapOffset + fnta::Input::Start; return 1;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK: *out.begin() = remapOffset + fnta::Input::L; return 1;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK: *out.begin() = remapOffset + fnta::Input::R; return 1;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: *out.begin() = remapOffset + fnta::Input::LB; return 1;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: *out.begin() = remapOffset + fnta::Input::RB; return 1;
    case SDL_CONTROLLER_BUTTON_DPAD_UP: *out.begin() = remapOffset + fnta::Input::Up; return 1;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN: *out.begin() = remapOffset + fnta::Input::Down; return 1;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT: *out.begin() = remapOffset + fnta::Input::Left; return 1;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: *out.begin() = remapOffset + fnta::Input::Right; return 1;
    // TODO: axes
    default: assert( !"unhandled enum" ); return 0;
    }
}

uint32_t Remapper::apply( Actuator::Source source, char32_t chr, std::span<char32_t> out ) const
{
    auto it = std::ranges::find_if( m_data, [chr, source]( const auto& p )
    {
        return (char32_t)p.m_userEnum == chr && source == p.m_max.source;
    } );
    if ( it == m_data.end() ) return 0;

    using Source = Actuator::Source;
    switch ( source ) {
    case Source::eKBM: {
        const SDL_Keycode vk = SDL_GetKeyFromScancode( (SDL_Scancode)it->m_max.typed.id );
        const char* keyName = SDL_GetKeyName( vk );
        if ( !keyName ) return 0;

        // TODO: utf32
        std::string_view sv{ keyName };
        assert( sv.size() + 2 < out.size() );
        auto kit = out.begin();
        *kit = U'['; kit++;
        kit = std::transform( sv.begin(), sv.end(), kit, []( char c ) -> char32_t { return c; } );
        *kit = U']'; kit++;
        return (uint32_t)std::distance( out.begin(), kit );
    }
    case Source::eXBoxOne: return mapController( it->m_max, XBOX_REMAP_OFFSET, out );
    default: assert( !"unhandled enum" );
    }
    return 0;
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
