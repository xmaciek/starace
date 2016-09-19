#pragma once
#include <stdint.h>
#include <vector>
#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class ShaderPrivate;

class SHADER {
public:
    static bool init();

    static void pushMatrix();
    static void popMatrix();
    static void clearMatrix();
    static void syncMatrix();
    static void multMatrix( const float* matrix );


    static const double deg2rad;
    static void rotate( double angle, bool x,  bool y,  bool z );
    static void rotateRad( double angle,  bool x,  bool y,  bool z );
    static void translate( double x,  double y,  double z );
    static void scale( double x,  double y,  double z );

    static void setColor( double r,  double g,  double b,  double a );
    static void setColorArray( uint32_t index );

    static uint64_t makeBuffer( const std::vector<double> &array );
    static uint32_t makeBuffer( const double* array, uint32_t size );
    static void deleteBuffer( uint32_t buffer );

    static void setTextureCoord( uint32_t buffer );
    static void draw( uint32_t type, uint32_t buffer, uint32_t size );

    static void setOrtho( double minX,  double maxX,  double minY,  double maxY,  double minZ = -1.0,  double maxZ = 1.0 );
    static void setPerspective( double fovY, double ratio, double min, double max );

    static uint32_t getQuad( uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2 );
    static uint32_t getQuadTextureCoord( double x1, double y1, double x2, double y2 );

private:
    SHADER();
    SHADER( SHADER& );
    SHADER& operator=( SHADER& );

    static ShaderPrivate* ptr;

};
