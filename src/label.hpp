#pragma once

#include "Font.h"

struct Alignment {
    enum Horizontal { Left, Center, Right };
    enum Vertical { Top, Middle, Bottom };
};

class Label {
private:
    const Font* m_font;
    std::string m_text;
    double m_x, m_y;
    double m_xOffset, m_yOffset;
    bool m_isVisible;
    Alignment::Horizontal m_alignmentHorizontal;
    Alignment::Vertical m_alignmentVertical;
    void recalcOffset();

public:
    Label( const Font* font = 0, const std::string& str = std::string() );
    void setText( const std::string& str );
    std::string text() const;
    void clear();

    template<class T>
    Label& operator << ( const T& param ) {
        std::stringstream xx;
        xx << param;
        std::string t;
        std::getline( xx, t );
        m_text += t;
        recalcOffset();
        return *this;
    }

    void setAlignment( Alignment::Horizontal horizontal, Alignment::Vertical vertical = Alignment::Bottom );

    void setFont( const Font* f );
    void move( double x, double y );
    void setVisible( bool b );
    bool isVisible() const;

    void draw() const;
};
