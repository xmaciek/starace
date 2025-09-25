#include "game_options.hpp"

#include <algorithm>
#include <cassert>

Percent::operator std::pmr::u32string () const
{
    auto r = std::to_string( value );
    r.append( "%" );
    return std::pmr::u32string{ r.begin(), r.end() };
}

std::pmr::vector<Percent> Percent::makeRange( uint32_t step, uint32_t extra )
{
    uint32_t count = extra + 100 / step;
    std::pmr::vector<Percent> r( count );
    std::ranges::generate( r, [v=uint8_t(0), step]() mutable { auto r = v; v += step; return Percent{ r }; } );
    return r;
}

template <>
std::pmr::u32string OptionsGFX::toString( const AntiAlias& aa )
{
    using enum AntiAlias;
    switch ( aa ) {
    case eOff: return std::pmr::u32string{ g_uiProperty.localize( "off"_hash ) };
    case eFXAA: return U"FXAA";
    case eVRSAA: return U"VRSAA";
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
    case VSync::eOff: return std::pmr::u32string{ g_uiProperty.localize( "off"_hash ) };
    case VSync::eOn: return std::pmr::u32string{ g_uiProperty.localize( "on"_hash ) };
    case VSync::eMailbox: return U"MAILBOX";
    [[unlikely]] default:
        assert( !"unhandled enum" );
        return U"<BUG>";
    }
}

void OptionsGFX::ui2settings( GameSettings& gs ) const
{
    gs.antialias = m_antialiasUI.value();
    gs.resolution = m_resolutionUI.value();
    gs.gamma = m_gammaUI.value();
    gs.fullscreen = m_fullscreenUI.value();
    gs.vsync = m_vsyncUI.value();
    gs.fpsLimiter = m_fpsLimiterUI.value();
}

void OptionsGFX::settings2ui( const GameSettings& gs )
{
    m_antialiasUI.assign( gs.antialias );
    m_resolutionUI.assign( gs.resolution );
    m_gammaUI.assign( gs.gamma );
    m_fullscreenUI.assign( gs.fullscreen );
    m_vsyncUI.assign( gs.vsync );
    m_fpsLimiterUI.assign( gs.fpsLimiter );
}

bool OptionsGFX::hasChanges( const GameSettings& gs ) const
{
    return gs.antialias != m_antialiasUI.value()
        || gs.resolution != m_resolutionUI.value()
        || gs.gamma != m_gammaUI.value()
        || gs.fullscreen != m_fullscreenUI.value()
        || gs.vsync != m_vsyncUI.value()
        || gs.fpsLimiter != m_fpsLimiterUI.value()
        ;
}

void OptionsAudio::ui2settings( GameSettings& gs ) const
{
    copySecure( m_driverNameUI.value(), gs.audioDriverName );
    copySecure( m_deviceNameUI.value(), gs.audioDeviceName );
    gs.audioMaster = m_masterUI.value();
    gs.audioSFX = m_sfxUI.value();
    gs.audioUI = m_uiUI.value();
}


void OptionsAudio::settings2ui( const GameSettings& gs )
{
    m_driverNameUI.assign( gs.audioDriverName );
    m_deviceNameUI.assign( gs.audioDeviceName );
    m_masterUI.assign( gs.audioMaster );
    m_sfxUI.assign( gs.audioSFX );
    m_uiUI.assign( gs.audioUI );
}

bool OptionsAudio::hasChanges( const GameSettings& gs ) const
{
    return m_masterUI.value() != gs.audioMaster
        || m_sfxUI.value() != gs.audioSFX
        || m_uiUI.value() != gs.audioUI
        || m_driverNameUI.value() != gs.audioDriverName
        || m_deviceNameUI.value() != gs.audioDeviceName
    ;
}

void OptionsGame::ui2settings( GameSettings& gs ) const
{
    gs.gameLang = m_languageUI.value().id;
}

void OptionsGame::settings2ui( const GameSettings& gs )
{
    m_languageUI.assignIf( [id=gs.gameLang]( const auto& info ) { return info.id == id; } );
}

bool OptionsGame::hasChanges( const GameSettings& gs ) const
{
    return gs.gameLang != m_languageUI.value().id;
}
