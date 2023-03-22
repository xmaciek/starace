#version 450

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
    Afterglow a = afterglow[ gl_InstanceIndex ];
    vec3 vertex = vertexPos[ gl_VertexIndex ] * a.xyzs.w + a.xyzs.xyz;

    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( vertex, 1.0 );

    uvOut = vertexUV[ gl_VertexIndex ];
    colorOut = a.color;
    radiusOut = a.radius;
}
