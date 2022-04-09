#pragma once

#include <engine/math.hpp>
#include <renderer/texture.hpp>

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
    Texture m_atlasTexture{};

    math::vec4 m_colorA{};

public:
    inline Texture atlasTexture() const { return m_atlasTexture; }
    inline const LinearAtlas* atlas() const { return m_atlas; }

    inline const Font* fontSmall() const { return m_fontSmall; }
    inline const Font* fontMedium() const { return m_fontMedium; }
    inline const Font* fontLarge() const { return m_fontLarge; }

    inline math::vec4 colorA() const { return m_colorA; }

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
    };
};

} // namespace ui

inline ui::Property g_uiProperty{};
