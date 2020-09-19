#ifndef SA_H
#define SA_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef __linux__
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_ttf.h>
#endif

struct Vertex {
    GLdouble x = 0.0;
    GLdouble y = 0.0;
    GLdouble z = 0.0;

    Vertex() = default;
    Vertex( GLdouble X, GLdouble Y, GLdouble Z )
    : x( X )
    , y( Y )
    , z( Z )
    {
    }

    Vertex& operator+=( const Vertex& v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vertex operator+( const Vertex& a ) const
    {
        return Vertex( *this ) += a;
    };

    Vertex operator-( const Vertex& a ) const
    {
        Vertex v;
        v.x = x - a.x;
        v.y = y - a.y;
        v.z = z - a.z;
        return v;
    };

    Vertex operator*( const double& a ) const
    {
        Vertex v;
        v.x = x * a;
        v.y = y * a;
        v.z = z * a;
        return v;
    };
};

struct MapProto {
    GLuint enemies = 0;
    GLuint preview_image = 0;
    std::string name{ "unnamed map" };
    std::string texture_location{};
    std::string TOP{};
    std::string BOTTOM{};
    std::string LEFT{};
    std::string RIGHT{};
    std::string FRONT{};
    std::string BACK{};
    std::string preview_image_location{};
};

struct ModelProto {
    std::string name{ "Unnamed Jet" };
    std::string model_file{};
    std::string model_texture{};
    GLfloat scale = 1.0f;
};

// const GLdouble DELTATIME = 1.0/60;
const GLdouble DELTATIME = 0.016;
// #define PI 3.14159265
const GLdouble PI = M_PI;
const GLdouble DEG2RAD = PI / 180.0;
const GLdouble RAD2DEG = PI * 180.0;
GLdouble random_range( GLdouble a, GLdouble b );
GLfloat colorhalf( GLfloat col );

Vertex cross_product( const Vertex& a, const Vertex& b );
GLdouble dot_product( const Vertex& a, const Vertex& b );
GLdouble length_v( const Vertex& v );
void normalise_v( Vertex& v );
GLdouble distance_v( const Vertex& a, const Vertex& b );

#endif
