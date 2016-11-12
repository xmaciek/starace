#pragma once

#include "Font.h"
#include "widget.hpp"

class Label : public Widget {
private:
    const Font* m_font;
    std::string m_text;
    virtual void recalcAlignmentOffset();

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
        recalcAlignmentOffset();
        return *this;
    }


    void setFont( const Font* f );
    void draw() const;
};
