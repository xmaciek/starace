#include "game_options.hpp"

std::pmr::u32string OptionsGFX::toString( OptionsGFX::AntiAlias aa )
{
    using enum OptionsGFX::AntiAlias;
    switch ( aa ) {
    case eOff: return g_uiProperty.localize( "off"_hash );
    case eFXAA: return U"FXAA";
    [[unlikely]] default: return U"<BUG>";
    }
}