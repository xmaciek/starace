#version 450

const uint INSTANCES = 4;

struct ColorScheme {
    vec4 inner1;
    vec4 inner2;
    vec4 outter1;
    vec4 outter2;
};

layout( binding = 0 ) uniform ubo {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 modelOffset;
    vec4 zSizeCutoff[ INSTANCES ];
    ColorScheme colorScheme;
};

layout( location = 0 ) out vec2 fragUV;
layout( location = 1 ) out flat float fragCutoff;
layout( location = 2 ) out flat ColorScheme fragColor;

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
    const vec4 zsc = zSizeCutoff[ gl_InstanceIndex ];
    const float size = zsc.y;
    vec3 xyz = vertexPos[ gl_VertexIndex ];
    xyz.xy *= size;
    xyz.z += zsc.x;

    gl_Position = projectionMatrix
        * viewMatrix
        * modelMatrix
        * vec4( xyz + modelOffset, 1.0 );

    fragUV = vertexUV[ gl_VertexIndex ];
    fragCutoff = zsc.z;
    fragColor = colorScheme;
}
