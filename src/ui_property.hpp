#pragma once

#include <renderer/texture.hpp>

class Game;
class Font;

namespace ui {

class Property {
    friend Game;
    const Font* m_fontSmall = nullptr;
    const Font* m_fontMedium = nullptr;
    const Font* m_fontLarge = nullptr;
    Texture m_buttonTexture{};

public:
    inline Texture buttonTexture() const { return m_buttonTexture; }
    inline const Font* fontSmall() const { return m_fontSmall; }
    inline const Font* fontMedium() const { return m_fontMedium; }
    inline const Font* fontLarge() const { return m_fontLarge; }

};

}

inline ui::Property g_uiProperty{};
