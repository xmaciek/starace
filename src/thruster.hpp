#ifndef SA_THRUSTER
#define SA_THRUSTER

#include "circle.hpp"
#include "sa.hpp"

class Thruster {
public:
    ~Thruster() = default;
    Thruster( GLdouble Length, GLdouble Radiust );
    //   void Draw();
    void DrawAt( const GLdouble& X, const GLdouble& Y, const GLdouble& Z );
    void Update();
    void SetColor( GLuint Num, GLfloat* ColorData );
    void SetLength( const GLdouble& newLength );

private:
    GLdouble length = 0.0;
    GLdouble length_shorter = 0.0;
    GLdouble Len = 0.0;
    GLuint wiggle = 0;
    GLfloat color[ 4 ][ 4 ]{};
    Circle inner;
    Circle outer;
};

#endif
