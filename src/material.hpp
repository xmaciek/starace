#pragma once

#include <cstdint>

typedef struct Material_t {
    enum Type { Unknown, MultiColor, Texture };

    inline Material_t( uint32_t id = 0, Type t = Unknown ) : m_id( id ), m_type( t ) {};
    inline operator bool() const { return m_id && m_type != Unknown; }

    uint32_t m_id;
    Type m_type;
} Material;
