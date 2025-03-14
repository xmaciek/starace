#include "game_options.hpp"

#include <cassert>

template <>
std::pmr::u32string OptionsGFX::toString( const OptionsGFX::AntiAlias& aa )
{
    using enum OptionsGFX::AntiAlias;
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

void OptionsGFX::set()
{
    m_antialias = m_antialiasUI.value();
    m_resolution = m_resolutionUI.value();
    m_gamma = m_gammaUI.value();
    m_fullscreen = m_fullscreenUI.value();
    m_vsync = m_vsyncUI.value();
    m_fpsLimiter = m_fpsLimiterUI.value();
}

void OptionsGFX::restore()
{
    m_antialiasUI.assign( m_antialias );
    m_resolutionUI.assign( m_resolution );
    m_gammaUI.assign( m_gamma );
    m_fullscreenUI.assign( m_fullscreen );
    m_vsyncUI.assign( m_vsync );
    m_fpsLimiterUI.assign( m_fpsLimiter );
}

bool OptionsGFX::hasChanges() const
{
    return m_antialias != m_antialiasUI.value()
        || m_resolution != m_resolutionUI.value()
        || m_gamma != m_gammaUI.value()
        || m_fullscreen != m_fullscreenUI.value()
        || m_vsync != m_vsyncUI.value()
        || m_fpsLimiter != m_fpsLimiterUI.value()
        ;
}

void OptionsAudio::set()
{
    m_driverName = m_driverNameUI.value();
    m_deviceName = m_deviceNameUI.value();
    m_master = m_masterUI.value();
    m_sfx = m_sfxUI.value();
    m_ui = m_uiUI.value();
}

void OptionsAudio::restore()
{
    m_driverNameUI.assign( m_driverName );
    m_deviceNameUI.assign( m_deviceName );
    m_masterUI.assign( m_master );
    m_sfxUI.assign( m_sfx );
    m_uiUI.assign( m_ui );
}

bool OptionsAudio::hasChanges() const
{
    return m_masterUI.value() != m_master
        || m_sfxUI.value() != m_sfx
        || m_uiUI.value() != m_ui
        || m_driverNameUI.value() != m_driverName
        || m_deviceNameUI.value() != m_deviceName
    ;
}




