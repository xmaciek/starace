#pragma once

#include <ui/widget.hpp>
#include <ui/lockit.hpp>
#include <ui/sprite.hpp>

#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>
#include <shared/fixed_map.hpp>
#include <shared/hash.hpp>
#include <shared/pmr_pointer.hpp>
#include <input/actuator.hpp>

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
    PipelineSlot m_pipelineSpriteSequence{};
    PipelineSlot m_pipelineSpriteSequenceColors{};
    PipelineSlot m_pipelineGlow{};
    PipelineSlot m_pipelineBlurDesaturate{};
    using InputSource = input::Actuator::Source;
    InputSource m_inputSource{};

    const Font* m_fontSmall = nullptr;
    const Font* m_fontMedium = nullptr;
    const Font* m_fontLarge = nullptr;
    const Font* m_atlas = nullptr;
    std::pmr::vector<Lockit> m_lockit{};
    uint32_t m_currentLang = 0;
    FixedMap<Hash::value_type, ui::DataModel*, 64> m_dataModels{};
    FixedMap<Hash::value_type, std::function<void()>, 64> m_gameCallbacks{};

    UniquePointer<Widget> m_pendingModalWidget{};
    ResourceMap<Sprite> m_sprites{};


public:
    struct PendingComboBox{
        math::vec2 position{};
        math::vec2 size{};
        DataModel* model = nullptr;
        inline operator bool () const noexcept { return !!model; };
    };

    Texture atlasTexture() const;

    inline const Font* fontSmall() const { return m_fontSmall; }
    inline const Font* fontMedium() const { return m_fontMedium; }
    inline const Font* fontLarge() const { return m_fontLarge; }

    inline PipelineSlot pipelineSpriteSequence() const { return m_pipelineSpriteSequence; }
    inline PipelineSlot pipelineSpriteSequenceColors() const { return m_pipelineSpriteSequenceColors; }
    inline PipelineSlot pipelineGlow() const { return m_pipelineGlow; }
    inline PipelineSlot pipelineBlurDesaturate() const { return m_pipelineBlurDesaturate; }

    inline bool setInputSource( InputSource s ) { return std::exchange( m_inputSource, s ) != s; }
    inline InputSource inputSource() const { return m_inputSource; }

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

    inline const ui::Font* atlas() const { return m_atlas; }
    inline const ui::Font* font( Hash::value_type h ) const
    {
        switch ( h ) {
        case "small"_hash: return m_fontSmall;
        default: assert( !"font not found" ); [[fallthrough]];
        case "medium"_hash: return m_fontMedium;
        case "large"_hash: return m_fontLarge;
        }
    }

    inline math::vec4 color( Hash::value_type h ) const
    {
        switch ( h ) {
        case "green"_hash: return { 0.0275f, 1.0f, 0.075f, 1.0f };
        default: assert( !"unknown color" ); [[fallthrough]];
        case "white"_hash: return { 1.0f, 1.0f, 1.0f, 1.0f };
        }
    }

    std::function<void(std::pair<uint32_t, PipelineSlot>)> setupPipeline();

    void addSprites( const Font* );
    Sprite sprite( Hash::value_type ) const;

};

} // namespace ui

inline ui::Property g_uiProperty{};
