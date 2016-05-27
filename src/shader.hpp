#pragma once

#include <stack>

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class SHADER {
public:
    ~SHADER();
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

    static uint32_t makeBuffer( const double* array, uint32_t size );
    static void deleteBuffer( uint32_t buffer );

    static void setTextureCoord( uint32_t buffer );
    static void draw( uint32_t type, uint32_t buffer, uint32_t size );

    static void setOrtho( double minX,  double maxX,  double minY,  double maxY,  double minZ = -1.0,  double maxZ = 1.0 );
    static void setPerspective( double fovY, double ratio, double min, double max );

private:
    SHADER();
    SHADER( SHADER& );
    SHADER& operator=( SHADER& );

    uint32_t programID;
    uint32_t vertexArrayID;
    static SHADER* ptr;
    std::stack<glm::mat4> modelMatrix, viewMatrix, projectionMatrix;

    uint32_t modelMatrixLocation, viewMatrixLocation, projectionMatrixLocation;
    uint32_t vertexLocation, vertexUVlocation;
    uint32_t colorStanceLocation, colorValueLocation, colorArrayLocation;

};
