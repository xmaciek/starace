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
#include <tuple>

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
    const Font* m_atlas = nullptr;
    LocTable m_locTable{};
    FixedMapView<Hash::value_type, ui::DataModel*> m_dataModels{};
    FixedMapView<Hash::value_type, std::function<void()>> m_gameCallbacks{};

    PipelineSlot m_pipelineSpriteSequence{};
    PipelineSlot m_pipelineSpriteSequenceColors{};

public:
    struct PendingComboBox{
        math::vec2 position{};
        math::vec2 size{};
        DataModel* model = nullptr;
        inline operator bool () const noexcept { return !!model; };
    };
    PendingComboBox m_pendingComboBox{};

    Texture atlasTexture() const;

    inline const Font* fontSmall() const { return m_fontSmall; }
    inline const Font* fontMedium() const { return m_fontMedium; }
    inline const Font* fontLarge() const { return m_fontLarge; }

    inline PipelineSlot pipelineSpriteSequence() const { return m_pipelineSpriteSequence; }
    inline PipelineSlot pipelineSpriteSequenceColors() const { return m_pipelineSpriteSequenceColors; }

    inline std::pmr::u32string localize( Hash::value_type key ) const
    {
        const auto* value = m_locTable.find( key );
        if ( value ) {
            return *value;
        }
        //assert( !"missing loc key" );
        return U"<BUG:Missing loc key>";
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

    std::tuple<math::vec4, uint16_t, uint16_t, Texture> sprite( Hash::value_type ) const;

};

} // namespace ui

inline ui::Property g_uiProperty{};
