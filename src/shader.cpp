#include "shader.hpp"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <vector>
#include <string>
#include <stack>


#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SINGLE_COLOR 0
#define ARRAY_COLOR 1
#define TEXTURE_COLOR 2


class ShaderPrivate {
public:
    ShaderPrivate();
    uint32_t programID;
    uint32_t vertexArrayID;
    static ShaderPrivate* ptr;
    std::stack<glm::mat4> modelMatrix, viewMatrix, projectionMatrix;

    uint32_t modelMatrixLocation, viewMatrixLocation, projectionMatrixLocation;
    uint32_t vertexLocation, vertexUVlocation;
    uint32_t colorStanceLocation, colorValueLocation, colorArrayLocation;
};


ShaderPrivate* SHADER::ptr = 0;
const double SHADER::deg2rad = atan( 1 ) * 4.0 / 180.0;

static std::string getFileContent( const char fileName[] ) {
    std::string fileContent;
    std::ifstream fileStream( fileName, std::ios::in );
    if ( fileStream.is_open() ) {
        std::string line;
        while ( getline( fileStream, line ) ) {
            fileContent += line + "\n";
        }
        fileStream.close();
    } else {
        fprintf( stderr, "Unable to read file: %s", fileName );
    }
    return fileContent;
}

static bool compileShader( uint32_t shaderID, const std::string& shaderCode ) {
    // compile shader
    const char* sourceCodePtr = shaderCode.c_str();
    glShaderSource( shaderID, 1, &sourceCodePtr, 0 );
    glCompileShader( shaderID );

    // check compilation
    int result = GL_FALSE;
    int logLength = 0;
    glGetShaderiv( shaderID, GL_COMPILE_STATUS, &result );
    glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &logLength );
    if ( result == GL_FALSE && logLength > 0 ) {
        std::vector<char> shaderErrorMessage( logLength );
        glGetShaderInfoLog( shaderID, logLength, 0, &shaderErrorMessage[0] );
        fprintf( stderr, "%s\n", &shaderErrorMessage[0] );
    }
    return result != GL_FALSE;
}

static uint32_t loadShaders( const char vertexShader[], const char fragmentShader[] ) {
    uint32_t vertexShaderID = glCreateShader( GL_VERTEX_SHADER );
    uint32_t fragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

    std::string vertexShaderCode( getFileContent( vertexShader ) );
    std::string fragmentShaderCode( getFileContent( fragmentShader ) );

    fprintf( stderr, "Compiling vertex shader...\n" );
    if ( !compileShader( vertexShaderID, vertexShaderCode ) ) {
        return 0;
    }

    fprintf( stderr, "Compiling fragment shader...\n" );
    if ( !compileShader( fragmentShaderID, fragmentShaderCode ) ) {
        glDeleteShader( vertexShaderID );
        return 0;
    }

    // shaders ready? compile program
    fprintf( stderr, "Compiling program...\n" );
    uint32_t programID = glCreateProgram();
    glAttachShader( programID, vertexShaderID );
    glAttachShader( programID, fragmentShaderID );
    glLinkProgram( programID );

    int result = GL_FALSE;
    int logLength = 0;
    glGetProgramiv( programID, GL_LINK_STATUS, &result );
    glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &logLength );
    if ( result == GL_FALSE && logLength > 0 ) {
        std::vector<char> programErrorMessage( logLength );
        glGetProgramInfoLog( programID, logLength, 0, &programErrorMessage[0] );
        fprintf( stderr, "%s\n", &programErrorMessage[0] );
    }
    glDeleteShader( vertexShaderID );
    glDeleteShader( fragmentShaderID );

    if ( result == GL_FALSE ) {
        fprintf( stderr, "Failed to create shader\n" );
        return 0;
    }

    return programID;
}

ShaderPrivate::ShaderPrivate() {
    glGenVertexArrays( 1, &vertexArrayID );
    glBindVertexArray( vertexArrayID );

    programID = loadShaders( "vertexShader.vert", "fragmentShader.frag" );
    glUseProgram( programID );

    modelMatrixLocation = glGetUniformLocation( programID, "modelMatrix" );
    viewMatrixLocation = glGetUniformLocation( programID, "viewMatrix" );
    projectionMatrixLocation = glGetUniformLocation( programID, "projectionMatrix" );
//     vertexLocation = glGetUniformLocation( programID, "vertexPosition" );
    vertexLocation = 0;
    vertexUVlocation = 1;
//     ptr->verticesLocation = glGetUniformLocation( programID, "verticesUV" );
    colorStanceLocation = glGetUniformLocation( programID, "colorStance" );
    colorValueLocation = glGetUniformLocation( programID, "colorValue" );
//     colorArrayLocation = glGetUniformLocation( programID, "colorArray" );
    colorArrayLocation = 5;
    modelMatrix.push( glm::mat4( 1 ) );
    viewMatrix.push( glm::mat4( 1 ) );
    projectionMatrix.push( glm::mat4( 1 ) );
}

bool SHADER::init() {
    if ( !ptr ) {
        glewExperimental = true;
        if ( glewInit() != GLEW_OK ) {
            fprintf( stderr, "Failed to initialize GLEW\n" );
            return false;
        }

        ptr = new ShaderPrivate();
        glEnableVertexAttribArray( ptr->vertexLocation );
        return true;
    }
    fprintf( stderr, "Shader is already initialized\n" );
    return false;
}

void SHADER::pushMatrix() {
    assert( ptr );
    ptr->modelMatrix.push( ptr->modelMatrix.top() );
    ptr->viewMatrix.push( ptr->viewMatrix.top() );
    ptr->projectionMatrix.push( ptr->projectionMatrix.top() );
}

void SHADER::popMatrix() {
    assert( ptr );
    ptr->modelMatrix.pop();
    ptr->viewMatrix.pop();
    ptr->projectionMatrix.pop();
    syncMatrix();
}



void SHADER::translate( double x, double y, double z ) {
    assert( ptr );
    ptr->modelMatrix.top() = glm::translate( ptr->modelMatrix.top(), glm::vec3( x, y, z ) );
    syncMatrix();
}


void SHADER::rotate( double angle, bool x, bool y, bool z ) {
    assert( ptr );
    rotateRad( angle * deg2rad, x, y, z );
}

void SHADER::rotateRad( double angle, bool x, bool y, bool z ) {
    assert( ptr );
    ptr->modelMatrix.top() = glm::rotate( ptr->modelMatrix.top(), (float)angle, glm::vec3( x, y, z ) );
    syncMatrix();
}

void SHADER::scale( double x, double y, double z ) {
    assert( ptr );
    ptr->modelMatrix.top() = glm::scale( ptr->modelMatrix.top(), glm::vec3( x, y, z ) );
    syncMatrix();
}


void SHADER::setOrtho( double minX, double maxX, double minY, double maxY, double minZ, double maxZ ) {
    assert( ptr );
    ptr->projectionMatrix.top() = glm::ortho( minX, maxX, minY, maxY, minZ, maxZ );
    syncMatrix();
}

void SHADER::setPerspective( double fovY, double ratio, double min, double max ) {
    assert( ptr );
    ptr->projectionMatrix.top() = glm::perspective( fovY * deg2rad, ratio, min, max );
    syncMatrix();
}

void SHADER::syncMatrix() {
    assert( ptr );
    glUniformMatrix4fv( ptr->modelMatrixLocation, 1, GL_FALSE, glm::value_ptr( ptr->modelMatrix.top() ) );
    glUniformMatrix4fv( ptr->viewMatrixLocation, 1, GL_FALSE, glm::value_ptr( ptr->viewMatrix.top() ) );
    glUniformMatrix4fv( ptr->projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr( ptr->projectionMatrix.top() ) );
}

void SHADER::multMatrix( const float* matrix ) {
    assert( ptr );
    assert( matrix );
    ptr->modelMatrix.top() = glm::make_mat4( matrix ) * ptr->modelMatrix.top();
    syncMatrix();
}


void SHADER::setColor( double r, double g, double b, double a ) {
    assert( ptr );
    glDisableVertexAttribArray( ptr->vertexUVlocation );
    glDisableVertexAttribArray( ptr->colorArrayLocation );
    glUniform1i( ptr->colorStanceLocation, SINGLE_COLOR );
    glUniform4f( ptr->colorValueLocation, (float)r, (float)g, (float)b, (float)a );
}
void SHADER::setColorArray( uint32_t index ) {
    assert( ptr );
    glDisableVertexAttribArray( ptr->vertexUVlocation );
    glEnableVertexAttribArray( ptr->colorArrayLocation );
    glUniform1i( ptr->colorStanceLocation, ARRAY_COLOR );
    glBindBuffer( GL_ARRAY_BUFFER, index );
    glVertexAttribPointer( ptr->colorArrayLocation, 4, GL_DOUBLE, GL_TRUE, 0, (void*)0 );
}

uint64_t SHADER::makeBuffer( const std::vector<double> &array )
{
    assert( !array.empty() );
    return SHADER::makeBuffer( &array[0], array.size() );
}

uint32_t SHADER::makeBuffer( const double* array, uint32_t size ) {
    assert( array );
    uint32_t location = 0;
    glGenBuffers( 1, &location );
    glBindBuffer( GL_ARRAY_BUFFER, location );
    glBufferData( GL_ARRAY_BUFFER, size * sizeof( double ), array, GL_STATIC_DRAW );
    return location;
}

void SHADER::deleteBuffer( uint32_t buffer ) {
    glDeleteBuffers( 1, &buffer );
}

void SHADER::setTextureCoord( uint32_t buffer ) {
    assert( ptr );
    glDisableVertexAttribArray( ptr->colorArrayLocation );
    glEnableVertexAttribArray( ptr->vertexUVlocation );
    glUniform1i( ptr->colorStanceLocation, TEXTURE_COLOR );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glVertexAttribPointer( ptr->vertexUVlocation, 2, GL_DOUBLE, GL_TRUE, 0, (void*)0 );
}


void SHADER::draw( uint32_t type, uint32_t buffer, uint32_t size ) {
    assert( ptr );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glVertexAttribPointer( ptr->vertexLocation, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0 );
    glDrawArrays( type, 0, size );
}

uint32_t SHADER::getQuad( uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2 ) {
    double arr[18] = { 0 };
    arr[0] = x1; arr[1] = y1;
    arr[3] = x2; arr[4] = y1;
    arr[6] = x2; arr[7] = y2;
    arr[9] = x1; arr[10] = y1;
    arr[12] = x2; arr[13] = y2;
    arr[15] = x1; arr[16] = y2;
    return SHADER::makeBuffer( arr, 18 );
}

uint32_t SHADER::getQuadTextureCoord( double x1, double y1, double x2, double y2 ) {
    double arr[12];
    arr[0] = x1; arr[1] = y1;
    arr[2] = x2; arr[3] = y1;
    arr[4] = x2; arr[5] = y2;
    arr[5] = x1; arr[7] = y1;
    arr[8] = x2; arr[9] = y2;
    arr[10] = x1; arr[11] = y2;
    return SHADER::makeBuffer( arr, 12 );
}
