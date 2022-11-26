#version 450

#extension GL_ARB_separate_shader_objects : enable

const uint INSTANCES = 320;

struct Particle {
    vec4 position;
    vec4 uvxywh;
    vec4 color;
};

layout( binding = 0 ) uniform ubo {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
    vec3 cameraUp;
    Particle particles[ INSTANCES ];
};

layout( location = 0 ) out vec2 fragmentUV;
layout( location = 1 ) out vec4 colorOut;

mat4 billboardModelMatrix( vec3 position )
{
    vec3 direction = normalize( cameraPosition - position );
    vec3 right = cross( cameraUp, direction );
    vec3 up = cross( direction, right );
    return mat4(
        vec4( right, 0 ),
        vec4( up, 0 ),
        vec4( direction, 0 ),
        vec4( position, 1 )
    );
}

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
    Particle particle = particles[ gl_InstanceIndex ];

    vec3 vertexPos = vertexPos[ gl_VertexIndex ] * particle.position.w;
    gl_Position = projectionMatrix
        * viewMatrix
        * billboardModelMatrix( particle.position.xyz )
        * vec4( vertexPos, 1.0 );

    fragmentUV = particle.uvxywh.xy + vertexUV[ gl_VertexIndex ] * particle.uvxywh.zw;
    colorOut = particle.color;
}
