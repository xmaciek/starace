#pragma once

#include <cstdint>

#include <GL/glew.h>

typedef struct Buffer_t {
    enum Type { Unknown,
        LineLoop = GL_LINE_LOOP,
        LineStrip = GL_LINE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN,
        Triangles = GL_TRIANGLES
    };

    inline Buffer_t( uint32_t id = 0, Type t = Unknown, uint32_t v = 0 ) :
        m_id( id ), m_type( t ), m_verticesCount( v ) {};

    inline operator bool () const { return m_id && m_type != Unknown && m_verticesCount; }
    inline operator uint32_t () const { return m_id; }

    uint32_t m_id;
    Type m_type;
    uint32_t m_verticesCount;
} Buffer;
