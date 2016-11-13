#pragma once

#include "buffer.hpp"
#include "label.hpp"
#include "widget.hpp"

class ValueBar : public Widget {
private:
    double m_value;
    mutable Buffer m_outline;
    mutable Buffer m_bar;
    Label m_text;

public:
    ValueBar();
    void setText( const std::string& text, const Font* font );
    void setValue( double value );
    virtual void draw() const;
};
