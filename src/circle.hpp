#ifndef SA_CIRCLE
#define SA_CIRCLE

#include "sa.hpp"

class Circle {
private:
    std::vector<GLdouble> m_x{};
    std::vector<GLdouble> m_y{};
    GLdouble m_radiust = 1.0;
    GLuint m_segments = 32;

    void init();

public:
    ~Circle() = default;
    Circle();
    Circle( GLuint segments, GLdouble radius );
    GLdouble x( GLuint ) const;
    GLdouble y( GLuint ) const;
    void setSegments( GLuint );
    void setRadius( GLdouble );
    GLuint segments() const;
    GLdouble radius() const;
};

#endif
