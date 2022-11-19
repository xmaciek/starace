#version 450

#extension GL_ARB_separate_shader_objects : enable

struct Afterglow {
    vec4 color;
    vec4 xyzs;
    float radius;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    Afterglow afterglow[ 4 ];
};

layout( location = 0 ) out vec4 colorOut;
layout( location = 1 ) out vec2 uvOut;
layout( location = 2 ) out float radiusOut;


const vec3 vertexPos[ 6 ] = {
    vec3( -1.0, -1.0, 0.0 ),
    vec3( -1.0, 1.0, 0.0 ),
    vec3( 1.0, 1.0, 0.0 ),
    vec3( 1.0, 1.0, 0.0 ),
    vec3( 1.0, -1.0, 0.0 ),
    vec3( -1.0, -1.0, 0.0 )
};

const vec2 vertexUV[ 6 ] = {
    vec2( 0.0, 0.0 ),
    vec2( 0.0, 1.0 ),
    vec2( 1.0, 1.0 ),
    vec2( 1.0, 1.0 ),
    vec2( 1.0, 0.0 ),
    vec2( 0.0, 0.0 )
};

void main()
{
    const uint instanceID = gl_VertexIndex / 6;
    const uint vertexID = gl_VertexIndex % 6;

    Afterglow a = afterglow[ instanceID ];
    vec3 vertex = vertexPos[ vertexID ] * a.xyzs.w + a.xyzs.xyz;

    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertex, 1.0 );

    uvOut = vertexUV[ vertexID ];
    colorOut = a.color;
    radiusOut = a.radius;
}
