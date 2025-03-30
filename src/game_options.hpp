#pragma once

#include "ui_options_models.hpp"

#include <shared/hash.hpp>
#include <ui/var.hpp>
#include <renderer/renderer.hpp>

#include <string>
#include <memory_resource>

enum class AntiAlias : uint32_t {
    eOff,
    eFXAA,
    eVRSAA,
};

struct GameSettings {
    static constexpr inline const uint32_t MAGIC = 'GGFC';
    static constexpr inline const uint32_t VERSION = 1;
    uint32_t magic = MAGIC;
    uint32_t version = VERSION;

    DisplayMode resolution{};
    float gamma = 2.2f;
    AntiAlias antialias = AntiAlias::eOff;
    VSync vsync = VSync::eOn;
    bool fullscreen = 1;
    bool fpsLimiter = 1;

    char audioDeviceName[ 256 ]{};
    char audioDriverName[ 256 ]{};
    float audioMaster = 1.0f;
    float audioSFX = 1.0f;
    float audioUI = 1.0f;

    char32_t gameLang[ 64 ]{};

    inline operator std::span<const uint8_t> () const
    {
        return std::span{ reinterpret_cast<const uint8_t*>( this ), sizeof( *this ) };
    }
};


struct OptionsGFX {
    template <typename T> static std::pmr::u32string toString( const T& );

    ui::Option<AntiAlias> m_antialiasUI{};
    ui::Option<DisplayMode> m_resolutionUI{};
    ui::Option<bool> m_fullscreenUI{ 1 };
    ui::Option<VSync> m_vsyncUI{};
    ui::Option<bool> m_fpsLimiterUI{ 1 };
    ui::Option<float> m_gammaUI{ 18,
        std::pmr::vector<float>{ 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f
            , 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f
            , 2.0f, 2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f
            , 3.0f, 3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f
            , 4.0f, 4.1f, 4.2f, 4.3f, 4.4f, 4.5f, 4.6f, 4.7f, 4.8f, 4.9f
        }
        , &::toString<float>
    };

    void settings2ui( const GameSettings& );
    void ui2settings( GameSettings& ) const;
    bool hasChanges( const GameSettings& ) const;
};

struct OptionsAudio {
    ui::Option<std::pmr::string> m_driverNameUI{ 0, []( const auto& s ) { return std::pmr::u32string{ s.begin(), s.end() }; } };
    ui::Option<std::pmr::string> m_deviceNameUI{ 0, []( const auto& s ) { return std::pmr::u32string{ s.begin(), s.end() }; } };
    ui::Option<float> m_masterUI{ 0
        , std::pmr::vector<float>{ 1.0f, 0.95f, 0.9f, 0.85f, 0.8f, 0.75f, 0.7f, 0.65f, 0.6f, 0.55f, 0.5f, 0.45f, 0.4f, 0.35f, 0.3f, 0.25f, 0.2f, 0.15f, 0.1f, 0.05f, 0.0f }
        , &toString<float>
    };
    ui::Option<float> m_sfxUI{ 0
        , std::pmr::vector<float>{ 1.0f, 0.95f, 0.9f, 0.85f, 0.8f, 0.75f, 0.7f, 0.65f, 0.6f, 0.55f, 0.5f, 0.45f, 0.4f, 0.35f, 0.3f, 0.25f, 0.2f, 0.15f, 0.1f, 0.05f, 0.0f }
        , &toString<float>
    };
    ui::Option<float> m_uiUI{ 0
        , std::pmr::vector<float>{ 1.0f, 0.95f, 0.9f, 0.85f, 0.8f, 0.75f, 0.7f, 0.65f, 0.6f, 0.55f, 0.5f, 0.45f, 0.4f, 0.35f, 0.3f, 0.25f, 0.2f, 0.15f, 0.1f, 0.05f, 0.0f }
        , &toString<float>
    };

    void settings2ui( const GameSettings& );
    void ui2settings( GameSettings& ) const;
    bool hasChanges( const GameSettings& ) const;
};

struct OptionsGame {
    ui::Option<std::pmr::u32string> m_languageUI{};

    void settings2ui( const GameSettings& );
    void ui2settings( GameSettings& ) const;
    bool hasChanges( const GameSettings& ) const;
};

struct OptionsCustomize {
    ui::GenericDataModel m_jet{};
    ui::GenericDataModel m_weaponPrimary{};
    ui::GenericDataModel m_weaponSecondary{};
};

struct GameplayUIData {
    ui::Var<float> m_playerHP{ 0.0f };
    ui::Var<float> m_jetSpeed{ 0.0f };
    ui::Var<float> m_playerReloadPrimary{ 0.0f };
    ui::Var<float> m_playerReloadSecondary{ 0.0f };
    ui::Var<uint32_t> m_playerWeaponPrimaryCount{ 0 };
    ui::Var<uint32_t> m_playerWeaponSecondaryCount{ 0 };
    ui::Var<Hash::value_type> m_playerWeaponIconPrimary{ 0 };
    ui::Var<Hash::value_type> m_playerWeaponIconSecondary{ 0 };
};

inline void copySecure( const auto& string, auto& arr )
{
    const auto size = std::min( string.size(), std::size( arr ) );
    auto it = std::copy_n( string.begin(), size, std::begin( arr ) );
    std::fill( it, std::end( arr ), 0 );
}

inline bool validate( auto& value, const auto& container )
{
    return std::find( container.begin(), container.end(), value ) != container.end();
}
