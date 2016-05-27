#version 330 core

#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 vertexPosition;
layout( location = 1 ) in vec2 vertexUV;
layout( location = 2 ) in vec3 vertexNormals;

/*layout(location = 3)*/ uniform int colorStance;
/*layout(location = 4)*/ uniform vec4 colorValue;
layout( location = 5 ) in vec4 colorArray;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

flat out int fragmentStance;
out vec4 fragmentColor;
out vec2 fragmentUV;

void colorFunction() {
    fragmentStance = 0;
    fragmentColor = colorValue;
}

void colorArrayFunction() {
    fragmentStance = 0;
    fragmentColor = vec4( colorArray[0], colorArray[1], colorArray[2], colorArray[3] );
}

void colorTextureFunction() {
    fragmentStance = 1;
    fragmentUV = vertexUV;
    fragmentColor = vec4( 1, 1, 1, 1 );
}


void main() {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4( vertexPosition, 1 );
    switch ( colorStance ) {
        case 0: colorFunction(); break;
        case 1: colorArrayFunction(); break;
        case 2: colorTextureFunction(); break;
        default: fragmentColor = vec4( 1, 1, 1, 1 ); break;
    }
}
