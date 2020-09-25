#pragma once

#include "update_context.hpp"

class Reactor {
    float m_capacity = 100.0f;
    float m_current = 100.0f;
    float m_generation = 60.0f;

public:
    bool consume( float );
    float power() const;
    void update( const UpdateContext& );
};
