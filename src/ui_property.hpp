#pragma once

#include "ui_localize.hpp"
#include "utils.hpp"

#include <engine/math.hpp>
#include <renderer/texture.hpp>

#include <cassert>

class Game;
class Font;
class LinearAtlas;

namespace ui {

class Property {
    friend Game;
    const Font* m_fontSmall = nullptr;
    const Font* m_fontMedium = nullptr;
    const Font* m_fontLarge = nullptr;
    const LinearAtlas* m_atlas = nullptr;
    const LocTable* m_locTable = nullptr;
    Texture m_atlasTexture{};

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

    inline math::vec4 colorA() const { return m_colorA; }

    inline std::pmr::u32string localize( Hash::value_type key ) const
    {
        assert( m_locTable );
        const auto* value = (*m_locTable)[ key ];
        return value ? *value : ( U"LOC:" + intToUTF32( key ) );
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
