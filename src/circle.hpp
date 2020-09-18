#ifndef SA_CIRCLE
#define SA_CIRCLE

#include "sa.hpp"

class Circle {
public:
    Circle();
    Circle( GLuint Segments, GLdouble Radiust );
    ~Circle();
    GLdouble GetX( GLuint a );
    GLdouble GetY( GLuint a );
    void SetSegments( GLuint Segments );
    void SetRadiust( GLdouble Radiust );
    GLuint GetSegments();
    GLdouble GetRadiust();

private:
    GLdouble DEGinRAD = 0.0;
    GLdouble radiust = 0.0;
    GLuint segments = 0;
    std::vector<GLdouble> X{};
    std::vector<GLdouble> Y{};
    void init();
};

#endif
