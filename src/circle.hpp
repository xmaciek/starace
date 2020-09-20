#ifndef SA_CIRCLE
#define SA_CIRCLE

#include "sa.hpp"

class Circle {
public:
    ~Circle() = default;
    Circle();
    Circle( GLuint Segments, GLdouble Radiust );
    GLdouble GetX( GLuint a ) const;
    GLdouble GetY( GLuint a ) const;
    void SetSegments( GLuint Segments );
    void SetRadiust( GLdouble Radiust );
    GLuint GetSegments() const;
    GLdouble GetRadiust() const;

private:
    std::vector<GLdouble> m_x{};
    std::vector<GLdouble> m_y{};
    GLdouble m_radiust = 1.0;
    GLuint m_segments = 32;
    void init();
};

#endif
