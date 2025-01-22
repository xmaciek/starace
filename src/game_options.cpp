#include "game_options.hpp"

#include <cassert>

template <>
std::pmr::u32string OptionsGFX::toString( const OptionsGFX::AntiAlias& aa )
{
    using enum OptionsGFX::AntiAlias;
    switch ( aa ) {
    case eOff: return g_uiProperty.localize( "off"_hash );
    case eFXAA: return U"FXAA";
    [[unlikely]] default:
        assert( !"unhandled enum" );
        return U"<BUG>";
    }
}

template <>
std::pmr::u32string OptionsGFX::toString( const DisplayMode&  dm )
{
    return intToUTF32( dm.width ) + U" x " + intToUTF32( dm.height );
}

template <>
std::pmr::u32string OptionsGFX::toString( const VSync& v )
{
    switch ( v ) {
    case VSync::eOff: return g_uiProperty.localize( "off"_hash );
    case VSync::eOn: return g_uiProperty.localize( "on"_hash );
    case VSync::eMailbox: return U"MAILBOX";
    [[unlikely]] default:
        assert( !"unhandled enum" );
        return U"<BUG>";
    }
}
