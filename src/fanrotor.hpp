#pragma once

#include "buffer.hpp"
#include "widget.hpp"
#include "shader.hpp"

class FanRotor : public Widget {
private:
    // for now assignment of Buffers happens in draw() call
    mutable Buffer m_fanCircle;
    mutable Buffer m_fanPetals;

    double m_rotation;

public:
    FanRotor();
    void setRotation( double rotation );
    virtual void draw() const;
};
