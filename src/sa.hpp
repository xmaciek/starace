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

using namespace std;

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
    GLuint enemies;
    GLuint preview_image;
    string name;
    string texture_location;
    string TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK;
    string preview_image_location;
    MapProto()
    {
        enemies = 0;
        preview_image = 0;
        name = "unnamed map";
        TOP = BOTTOM = LEFT = RIGHT = FRONT = BACK = "";
        texture_location = "";
        preview_image_location = "";
    };
};

struct ModelProto {
    string name;
    string model_file;
    string model_texture;
    GLfloat scale;
    ModelProto()
    {
        //     cout<<"Adding";
        scale = 1;
        name = "Unnamed Jet";
        model_file = "";
        model_texture = "";
    };
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
