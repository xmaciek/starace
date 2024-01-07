#version 450

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 position;
    vec3 displacement;
    vec4 color1;
    vec4 color2;
};

layout( location = 0 ) out flat vec4 colorOut1;
layout( location = 1 ) out flat vec4 colorOut2;
layout( location = 2 ) out float u;

const vec3 vertexPos[ 12 ] = {
    vec3( 0.0, -1.0, 0.0 ),
    vec3( 0.0, -1.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ),
    vec3( 0.0, 1.0, 1.0 ),
    vec3( 0.0, 1.0, 0.0 ),
    vec3( 0.0, -1.0, 0.0 ),

    vec3( -1.0, 0.0, 0.0 ),
    vec3( 1.0, 0.0, 0.0 ),
    vec3( 1.0, 0.0, 1.0 ),
    vec3( 1.0, 0.0, 1.0 ),
    vec3( -1.0, 0.0, 1.0 ),
    vec3( -1.0, 0.0, 0.0 ),
};

void main()
{
    vec3 mult = vertexPos[ gl_VertexIndex ];
    vec3 pos = position + mult * displacement;
    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( pos, 1.0 );

    colorOut1 = color1;
    colorOut2 = color2;
    u = ( 1.0 + mult.x + mult.y ) * 0.5;
}
