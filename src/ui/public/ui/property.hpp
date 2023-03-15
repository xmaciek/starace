#pragma once

#include <engine/math.hpp>
#include <renderer/texture.hpp>
#include <renderer/pipeline.hpp>
#include <shared/fixed_map.hpp>
#include <shared/hash.hpp>


#include <cassert>
#include <memory_resource>
#include <string>

class Game;
class Font;
class LinearAtlas;
using LocTable = FixedMap<Hash::value_type, std::pmr::u32string, 64>;

namespace ui {
struct DataModel;

class Property {
    friend Game;
    const Font* m_fontSmall = nullptr;
    const Font* m_fontMedium = nullptr;
    const Font* m_fontLarge = nullptr;
    const LinearAtlas* m_atlas = nullptr;
    const LocTable* m_locTable = nullptr;
    Texture m_atlasTexture{};

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
    inline const LinearAtlas* atlas() const { return m_atlas; }

    inline const Font* fontSmall() const { return m_fontSmall; }
    inline const Font* fontMedium() const { return m_fontMedium; }
    inline const Font* fontLarge() const { return m_fontLarge; }

    inline PipelineSlot pipelineSpriteSequence() const { return m_pipelineSpriteSequence; }
    inline PipelineSlot pipelineSpriteSequenceRGBA() const { return m_pipelineSpriteSequenceRGBA; }
    inline PipelineSlot pipelineSpriteSequenceColors() const { return m_pipelineSpriteSequenceColors; }

    inline math::vec4 colorA() const { return m_colorA; }

    inline std::pmr::u32string localize( Hash::value_type key ) const
    {
        assert( m_locTable );
        const auto* value = (*m_locTable)[ key ];
        if ( value ) {
            return *value;
        }
        assert( !"missing loc key" );
        return U"<BUG:Missing loc key>";
    }

    inline std::pmr::u32string localize( std::string_view key ) const
    {
        assert( m_locTable );
        Hash hash{};
        const auto* value = (*m_locTable)[ hash( key ) ];
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
};


struct AtlasSprite {
    enum : uint32_t {
        eBackground,
        eArrowRight,
        eArrowLeft,
        eTopLeft,
        eTop,
        eTopRight,
        eLeft,
        eMid,
        eRight,
        eBotLeft,
        eBot,
        eBotRight,
        eBotLeft2,
        eBotRight2,
        count
    };
};

} // namespace ui

inline ui::Property g_uiProperty{};
