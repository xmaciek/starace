#pragma once

#include <cmath>
#include <cstdint>
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

struct MapProto {
    uint32_t enemies = 0;
    uint32_t preview_image = 0;
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
    float scale = 1.0f;
};

const double PI = M_PI;
const double DEG2RAD = PI / 180.0;
const double RAD2DEG = PI * 180.0;

double randomRange( double a, double b );
double colorHalf( double col );
