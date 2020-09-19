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
    GLdouble radiust = 1.0;
    GLuint segments = 32;
    std::vector<GLdouble> X{};
    std::vector<GLdouble> Y{};
    void init();
};

#endif
