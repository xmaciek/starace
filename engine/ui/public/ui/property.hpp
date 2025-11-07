#pragma once

#include <ui/widget.hpp>
#include <ui/lockit.hpp>
#include <ui/sprite.hpp>
#include <ui/font_map.hpp>
#include <ui/screen.hpp>

#include <audio/audio.hpp>
#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>
#include <shared/fixed_map.hpp>
#include <shared/hash.hpp>
#include <shared/pmr_pointer.hpp>
#include <input/actuator.hpp>
#include <input/remapper.hpp>

#include <cassert>
#include <functional>
#include <memory_resource>
#include <string>
#include <tuple>
#include <engine/resource_map.hpp>

class Game;

namespace ui {
class DataModel;
class Font;

class Property {
    friend Game;
    using InputSource = input::Actuator::Source;
    InputSource m_inputSource{};
    input::Remapper* m_remapper = nullptr;
    FontMap m_fontMap{};
    std::pmr::vector<Lockit> m_lockit{};
    std::pmr::list<Screen> m_screens{};
    Screen* m_currentScreen = nullptr;
    math::vec2 m_viewport{};
    uint32_t m_currentLang = 0;
    FixedMap<Hash::value_type, ui::DataModel*, 64> m_dataModels{};
    FixedMap<Hash::value_type, std::function<void()>, 64> m_gameCallbacks{};

    UniquePointer<Widget> m_pendingModalWidget{};
    ResourceMap<Sprite> m_sprites{};
    const ResourceMap<Texture>* m_textures = nullptr;
    const ResourceMap<PipelineSlot>* m_materials = nullptr;
    const ResourceMap<Audio::Slot>* m_sounds = nullptr;
    Audio* m_audio = nullptr;


public:
    struct PendingComboBox{
        math::vec2 position{};
        math::vec2 size{};
        DataModel* model = nullptr;
        inline operator bool () const noexcept { return !!model; };
    };

    inline bool setInputSource( InputSource s ) { return std::exchange( m_inputSource, s ) != s; }
    inline InputSource inputSource() const { return m_inputSource; }
    inline auto remapper() const { return m_remapper; }
    inline auto& fontMap() const { return m_fontMap; }

    inline std::u32string_view localize( Hash::value_type key ) const
    {
        assert( !m_lockit.empty() );
        if ( m_lockit.empty() ) [[unlikely]] return U"<no lockit>";

        assert( m_currentLang < m_lockit.size() );
        return m_lockit[ m_currentLang ].find( key );
    }

    inline void requestModalWidget( UniquePointer<Widget>&& w )
    {
        m_pendingModalWidget = std::move( w );
    }

    inline UniquePointer<Widget> pendingModalWidget()
    {
        return std::move( m_pendingModalWidget );
    }

    inline std::function<void()> gameCallback( Hash::value_type h ) const
    {
        auto callback = m_gameCallbacks.find( h );
        assert( callback );
        assert( *callback );
        return *callback;
    }

    inline void addCallback( Hash::value_type h, std::function<void()>&& f )
    {
        m_gameCallbacks.insert( h, std::move( f ) );
    }

    inline ui::DataModel* dataModel( Hash::value_type h ) const
    {
        auto dataModel = m_dataModels.find( h );
        assert( dataModel );
        assert( *dataModel );
        return *dataModel;
    }

    inline void addDataModel( Hash::value_type h, DataModel* model )
    {
        m_dataModels.insert( h, model );
    }

    inline const Font* font( Hash::value_type h ) const
    {
        return m_fontMap.findFont( h );
    }

    inline math::vec4 color( Hash::value_type h ) const
    {
        switch ( h ) {
        case "green"_hash: return { 0.0275f, 1.0f, 0.075f, 1.0f };
        default: assert( !"unknown color" ); [[fallthrough]];
        case "white"_hash: return { 1.0f, 1.0f, 1.0f, 1.0f };
        }
    }

    void addSprites( const Font* );
    Sprite sprite( Hash::value_type ) const;
    void playSound( Hash::value_type );
    inline Texture findTexture( Hash::value_type hh ) { return m_textures->find( hh ); }
    inline PipelineSlot findMaterial( Hash::value_type hh ) { return m_materials->find( hh ); }
    inline Screen* currentScreen() const { return m_currentScreen; }
    void changeScreen( Hash::value_type, math::vec2 );
    void changeScreen( Hash::value_type );
    uint32_t changeLockit( std::array<char, 8> );

    void loadATLAS( std::span<const uint8_t> );
    void loadFNTA( std::span<const uint8_t> );
    void loadUI( std::span<const uint8_t> );
};

} // namespace ui

inline ui::Property g_uiProperty{};
