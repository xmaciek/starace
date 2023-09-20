#pragma once

#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>
#include <shared/fixed_map.hpp>
#include <shared/hash.hpp>

#include <cassert>
#include <functional>
#include <memory_resource>
#include <string>

class Game;

namespace ui {
class DataModel;
class Atlas;
class Font;
using LocTable = FixedMapView<Hash::value_type, std::pmr::u32string>;

class Property {
    friend Game;
    const Font* m_fontSmall = nullptr;
    const Font* m_fontMedium = nullptr;
    const Font* m_fontLarge = nullptr;
    const Atlas* m_atlas = nullptr;
    LocTable m_locTable{};
    Texture m_atlasTexture{};
    FixedMapView<Hash::value_type, ui::DataModel*> m_dataModels{};
    FixedMapView<Hash::value_type, std::function<void()>> m_gameCallbacks{};

    PipelineSlot m_pipelineSpriteSequence{};
    PipelineSlot m_pipelineSpriteSequenceRGBA{};
    PipelineSlot m_pipelineSpriteSequenceColors{};

    math::vec4 m_colorA{};


public:
    struct PendingComboBox{
        math::vec2 position{};
        math::vec2 size{};
        DataModel* model = nullptr;
        inline operator bool () const noexcept { return !!model; };
    };
    PendingComboBox m_pendingComboBox{};

    inline Texture atlasTexture() const { return m_atlasTexture; }
    inline const Atlas* atlas() const { return m_atlas; }

    inline const Font* fontSmall() const { return m_fontSmall; }
    inline const Font* fontMedium() const { return m_fontMedium; }
    inline const Font* fontLarge() const { return m_fontLarge; }

    inline PipelineSlot pipelineSpriteSequence() const { return m_pipelineSpriteSequence; }
    inline PipelineSlot pipelineSpriteSequenceRGBA() const { return m_pipelineSpriteSequenceRGBA; }
    inline PipelineSlot pipelineSpriteSequenceColors() const { return m_pipelineSpriteSequenceColors; }

    inline math::vec4 colorA() const { return m_colorA; }

    inline std::pmr::u32string localize( Hash::value_type key ) const
    {
        const auto* value = m_locTable.find( key );
        if ( value ) {
            return *value;
        }
        //assert( !"missing loc key" );
        return U"<BUG:Missing loc key>";
    }

    inline std::pmr::u32string localize( std::string_view key ) const
    {
        Hash hash{};
        const auto* value = m_locTable.find( hash( key ) );
        return value ? *value : ( U"LOC:" + std::pmr::u32string{ key.begin(), key.end() } );
    }

    inline void requestModalComboBox( math::vec2 position, math::vec2 size, DataModel* model )
    {
        m_pendingComboBox = { position, size, model };
    }

    inline PendingComboBox pendingModalComboBox() const
    {
        return m_pendingComboBox;
    }

    inline std::function<void()> gameCallback( Hash::value_type h ) const
    {
        auto callback = m_gameCallbacks.find( h );
        assert( callback );
        assert( *callback );
        return *callback;
    }

    inline ui::DataModel* dataModel( Hash::value_type h ) const
    {
        auto dataModel = m_dataModels.find( h );
        assert( dataModel );
        assert( *dataModel );
        return *dataModel;
    }
};

} // namespace ui

inline ui::Property g_uiProperty{};
